#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "compiler.h"
#include "scanner.h"

typedef struct
{
    Token current;
    Token previous;
    bool hadError;  // Flag to alert an error.
    bool panicMode; // Flag to enter in panic mode and re-sync the parser with the code.
} Parser;

// Lowes to heighest precedense.
typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR,         // or
    PREC_AND,        // and
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM,       // + -
    PREC_FACTOR,     // * /
    PREC_UNARY,      // ! -
    PREC_CALL,       // . ()
    PREC_PRIMARY
} Precedence;

Parser parser;
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
        return; // Supress erros if we already had one.
    parser.panicMode = true;
    fprintf(stderr, "[loine &d] Error", token->line);
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
        fprintf(stderr, "at '%.*s'", token->length, token->start);
    }
    fprintf(stderr, ":%s\n", message);
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
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX)
    {
        error("To many constants in one chunk.");
        return 0;
    }
    return (uint8_t)constant;
}

// Loads a value.
static void emitConstant(Value value)
{
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void endCompiler()
{
    emitReturn();
}

// Grouping isn't a back-end useful, it's front-end syntactic sugar.
// Assuming that the initial ( has been consumed, it calls to expression to compile the expression.
static void grouping()
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

// Store a pointer and then emit the constant. Takes the lexeme and conver it to a double value.
static void number()
{
    double value = strtod(parser.previous.start, NULL);
    emitConstant(value);
}

// Prefix the expression.
static void unary()
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

    default:
        return; // Unreachable.
    }
}

/*
Starts at the current token and parses any expression at the given precedence level or higher.
*/
static void parsePrecedence(Precedence precedence)
{
}

static void expression()
{
    parsePrecedence(PREC_ASSIGNMENT);
}

bool compile(const char *source, Chunk *chunk)
{
    initScanner(source);
    compilingChunk = chunk;
    parser.hadError = false;
    parser.panicMode = false;
    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");
    endCompiler();
    return !parser.hadError;
}