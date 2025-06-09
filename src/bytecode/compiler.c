#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "hashutils.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

// ? Scope hashmap

static bool tokenEqual(const void *a_ptr, const void *b_ptr) {
    const Token *a = (const Token *)a_ptr;
    const Token *b = (const Token *)b_ptr;

    if (a->type != b->type) return false;

    if (a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

void freeTokenKey(void *key) {
    Token *token = (Token *)key;
    free(token);
}

#include "value_hashmap.h"
#define HASHMAP_NAME Scope
#define KEY_TYPE Token*
#define VALUE_TYPE void*
#define HASH_FUNC hashToken
#define EQUALS_FUNC tokenEqual
#include "hashmap_impl.h"


// ?


typedef struct
{
    Scope* scopes;
    int scopeDepth;
} Compiler;

Parser parser;
Compiler *current = NULL;
Chunk *compilingChunk;

// Returns the current compiling chunk.
static Chunk *currentChunk()
{
    return compilingChunk;
}

/*
1. Prints where the error occurred.
2. Shows the lexeme if it's human-readable.
3. Then prints the error message.
4. Sets the hadError flag.
*/
static void errorAt(Token *token, const char *message)
{
    if (parser.panicMode)
        return; // Suppress errors if we already had one.

    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF)
    {
        fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_ERROR)
    {
        // Nothing.
    }
    else
    {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

/*
Extracts the location of the current token to tell the uses where happened the error and then calls
to errorAt.
*/
static void error(const char *message)
{
    errorAt(&parser.previous, message);
}

/*
It tells the user what happened
*/
static void errorAtCurrent(const char *message)
{
    errorAt(&parser.current, message);
}

/*
It steps forward through the toke nstream.
It takes the old current token and stores it in a previous field (to get the lexeme after math a token)
Ask the scanner for next token and stores it for later use.
*/
static void advance()
{
    parser.previous = parser.current;
    for (;;)
    {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR)
            break;

        errorAtCurrent(parser.current.start);
    }
}

/*
Reads the next token and validates that token has expected type. If not, it reports an error.
*/
static void consume(TokenType type, const char *message)
{
    if (parser.current.type == type)
    {
        advance();
        return;
    }
    errorAtCurrent(message);
}

// The check() function returns true if the current token has the given type.
static bool check(TokenType type)
{
    return parser.current.type == type;
}

/*
You may recognize it from jlox.
If the current token has the given type, we consume the token and return true.
Otherwise we leave the token alone and return false.
*/
static bool match(TokenType type)
{
    if (!check(type))
        return false;
    advance();
    return true;
}

// Adds a byte to the chunk
static void emitByte(uint8_t byte)
{
    writeChunk(currentChunk(), byte, parser.previous.line);
}

// Convenience function to emit opcode followed by a one-byte operand.
static void emitBytes(uint8_t byte1, uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn()
{
    emitByte(OP_RETURN);
}

// Makes sure that whe don't add more than 256 constant in a chunk.
static uint8_t makeConstant(Value value)
{
    // Linear search to avoid duplicate constants in constant pool.
    Chunk *chunk = currentChunk();
    for (int i = 0; i < chunk->constants.count; i++)
    {
        if (valuesEqual(chunk->constants.values[i], value))
        {
            return (uint8_t)i;
        }
    }

    // Normal behaviour.
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX)
    {
        error("Too many constants in one chunk.");
        return 0;
    }
    return (uint8_t)constant;
}

// Loads a value.
static void emitConstant(Value value)
{
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void initCompiler(Compiler *compiler)
{
    compiler->scopes = 0;
    compiler->scopeDepth = 0;
    current = compiler;
}

static void endCompiler()
{
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError)
    {
        disassembleChunk(currentChunk(), "code");
    }
#endif
}

static void beginScope()
{
    current->scopeDepth++;
    Scope* newScope = Scope_new(hashToken, tokenEqual, freeTokenKey, );  
    current->scopes[current->scopeDepth] = newScope;
}

static void endScope()
{
    current->scopeDepth--;
    while (current->localCount > 0 && current->locals[current->localCount - 1].depth > current->scopeDepth) 
    {
        emitByte(OP_POP);
        current->localCount--;
    }
    
}

static void expression();
static void statement();
static void declaration();
static ParseRule *getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static uint8_t identifierConstant(Token *name)
{
    // return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
    return makeConstant(copyString(name->start, name->length));
}

static bool identifiersEqual(Token *a, Token *b)
{
    if (a->length != b->length)
        return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Compiler* compiler, Token* name) {
    for (int i = compiler->localCount - 1; i >= 0; i--)
    {
        Local* local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name)) {
            if (local->depth == -1) {
                error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }

}

static void addLocal(Token name)
{
    if (current->localCount == UINT8_COUNT)
    {
        error("Too many local variables in function.");
        return;
    }
    Local *local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
}

static void declareVariable()
{
    if (current->scopeDepth == 0)
        return;

    Token *name = &parser.previous;
    for (int i = current->localCount - 1; i >= 0; i--)
    {
        Local *local = &current->locals[i];
        if (local->depth != 1 && local->depth < current->scopeDepth)
        {
            break;
        }
        if (identifiersEqual(name, &local->name))
        {
            error("Already a variable with this name in this scope.");
        }
    }

    addLocal(*name);
}

static uint8_t parseVariable(const char *errorMessage)
{
    consume(TOKEN_IDENTIFIER, errorMessage);

    declareVariable();
    if (current->scopeDepth > 0)
        return 0;

    return identifierConstant(&parser.previous);
}

static void markInitialized() {
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(uint8_t global)
{
    if (current->scopeDepth > 0)
    {
        markInitialized();
        return;
    }
    emitBytes(OP_DEFINE_GLOBAL, global);
}

/*
Compiles a binary operation expression.

This function is called after the left-hand operand has already been compiled
and the infix operator has been consumed. It parses and compiles the right-hand
operand using a higher precedence level to ensure left-associative behavior.

Once both operands are compiled, it emits the appropriate bytecode instruction
for the operator (e.g., OP_ADD for '+', OP_SUBTRACT for '-', etc.).

This allows multiple binary operators to be handled by a single function using
dynamic precedence lookup through getRule().
*/
static void binary(bool canAssing)
{
    TokenType operatorType = parser.previous.type;
    ParseRule *rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));
    switch (operatorType)
    {
    case TOKEN_PLUS:
        emitByte(OP_ADD);
        break;
    case TOKEN_MINUS:
        emitByte(OP_SUBTRACT);
        break;
    case TOKEN_STAR:
        emitByte(OP_MULTIPLY);
        break;
    case TOKEN_SLASH:
        emitByte(OP_DIVIDE);
        break;

    case TOKEN_BANG_EQUAL:
        emitBytes(OP_EQUAL, OP_NOT);
        break;
    case TOKEN_EQUAL_EQUAL:
        emitByte(OP_EQUAL);
        break;
    case TOKEN_GREATER:
        emitByte(OP_GREATER);
        break;
    case TOKEN_GREATER_EQUAL:
        emitBytes(OP_LESS, OP_NOT);
        break;
    case TOKEN_LESS:
        emitByte(OP_LESS);
        break;
    case TOKEN_LESS_EQUAL:
        emitBytes(OP_GREATER, OP_NOT);
        break;

    default:
        return; // Unreachable.
    }
}

static void literal(bool canAssing)
{
    switch (parser.previous.type)
    {
    case TOKEN_FALSE:
        emitByte(OP_FALSE);
        break;
    case TOKEN_NIL:
        emitByte(OP_NIL);
        break;
    case TOKEN_TRUE:
        emitByte(OP_TRUE);
        break;
    default:
        return;
    }
}

// Grouping isn't a back-end useful, it's front-end syntactic sugar.
// Assuming that the initial ( has been consumed, it calls to expression to compile the expression.
static void grouping(bool canAssing)
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

// Store a pointer and then emit the constant. Takes the lexeme and conver it to a double value.
static void number(bool canAssing)
{
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

// Takes string's characters from lexeme and wraps it in a Value then puts in the constant table.
static void string(bool canAssing)
{
    emitConstant(copyString(parser.previous.start + 1, parser.previous.length - 2));
}

static void namedVariable(Token name, bool canAssing)
{
    uint8_t getOp, setOp;
    uint8_t arg = resolveLocal(current, &name);
    if (arg!=-1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }
    if (canAssing && match(TOKEN_EQUAL))
    {
        expression();
        emitBytes(setOp, (uint8_t)arg);
    }
    else
    {
        emitBytes(getOp, (uint8_t)arg);
    }
}

static void variable(bool canAssing)
{
    namedVariable(parser.previous, canAssing);
}

// Prefix the expression.
static void unary(bool canAssing)
{
    TokenType operatorType = parser.previous.type;

    // Compile the operand.
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction.
    switch (operatorType)
    {
    case TOKEN_MINUS:
        emitByte(OP_NEGATE);
        break;

    case TOKEN_BANG:
        emitByte(OP_NOT);
        break;

    default:
        return; // Unreachable.
    }
}

static void ternary()
{
    parsePrecedence(PREC_TERNARY); // True body

    consume(TOKEN_COLON, "Expect : ':' after expression.");

    parsePrecedence(PREC_TERNARY); // False body
}

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = {grouping, NULL, PREC_NONE},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
    //  [TOKEN_INTERROGATION_OPEN] = {NULL, ternary, PREC_TERNARY}
    [TOKEN_LEFT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT] = {NULL, NULL, PREC_NONE},
    [TOKEN_MINUS] = {unary, binary, PREC_TERM},
    [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
    [TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
    [TOKEN_BANG] = {unary, NULL, PREC_NONE},
    [TOKEN_BANG_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_EQUAL_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER] = {variable, NULL, PREC_NONE},
    [TOKEN_STRING] = {string, NULL, PREC_NONE},
    [TOKEN_NUMBER] = {number, NULL, PREC_NONE},
    [TOKEN_AND] = {NULL, NULL, PREC_NONE},
    [TOKEN_CLASS] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE] = {literal, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_FUN] = {NULL, NULL, PREC_NONE},
    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL] = {literal, NULL, PREC_NONE},
    [TOKEN_OR] = {NULL, NULL, PREC_NONE},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER] = {NULL, NULL, PREC_NONE},
    [TOKEN_THIS] = {NULL, NULL, PREC_NONE},
    [TOKEN_TRUE] = {literal, NULL, PREC_NONE},
    [TOKEN_VAR] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

/*
Starts at the current token and parses any expression at the given precedence level or higher.
Reads the next tokend and looks the corresponding ParserRule. If not exists a caller it returns.
*/
static void parsePrecedence(Precedence precedence)
{
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL)
    {
        error("Expect expression.");
        return;
    }
    bool canAssing = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssing);
    while (precedence <= getRule(parser.current.type)->precedence)
    {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        if (infixRule != NULL)
        {
            infixRule(canAssing);
        }
        if (canAssing && match(TOKEN_EQUAL))
        {
            error("Invalid assignment target.");
        }
    }
}

/*
Returns the rule at the given index
*/
static ParseRule *getRule(TokenType type)
{
    return &rules[type];
}

static void expression()
{
    parsePrecedence(PREC_ASSIGNMENT);
}

static void block()
{
    while (!check(TOKEN_LEFT_BRACE) && !check(TOKEN_EOF))
    {
        declaration();
    }
    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void varDeclaration()
{
    uint8_t global = parseVariable("Expect variable name.");
    if (match(TOKEN_EQUAL))
    {
        expression();
    }
    else
    {
        emitByte(OP_NIL);
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    defineVariable(global);
}

static void synchronize()
{
    parser.panicMode = false;
    while (parser.current.type != TOKEN_EOF)
    {
        if (parser.previous.type == TOKEN_SEMICOLON)
            return;
        switch (parser.current.type)
        {
        case TOKEN_CLASS:
        case TOKEN_FUN:
        case TOKEN_VAR:
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_PRINT:
        case TOKEN_RETURN:
            return;
        default:;
        }

        advance();
    }
}

static void declaration()
{
    if (match(TOKEN_VAR))
    {
        varDeclaration();
    }
    else
    {
        statement();
    }
    if (parser.panicMode)
        synchronize();
}

static void expressionStatement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

static void printStatement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

static void statement()
{
    if (match(TOKEN_PRINT))
    {
        printStatement();
    }
    else if (match(TOKEN_LEFT_BRACE))
    {
        beginScope();
        expressionStatement();
        endScope();
    }
}

bool compile(const char *source, Chunk *chunk)
{
    initScanner(source);
    Compiler compiler;
    initCompiler(&compiler);
    compilingChunk = chunk;
    parser.hadError = false;
    parser.panicMode = false;
    advance();
    while (!match(TOKEN_EOF))
    {
        declaration();
    }

    endCompiler();
    return !parser.hadError;
}