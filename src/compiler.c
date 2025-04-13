#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "compiler.h"
#include "scanner.h"

typedef struct
{
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

Parser parser;

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
static void consume(TokenType type, const char *message)
{
    if (parser.current.type == type)
    {
        advance();
        return;
    }
    errorAtCurrent(message);
}
void compile(const char *source, Chunk *Chunk);
{
    initScanner(source);
    parser.hadError = false;
    parser.panicMode = false;
    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.") return !parser.hadError;
}