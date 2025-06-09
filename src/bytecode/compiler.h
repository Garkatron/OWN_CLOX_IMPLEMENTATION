#ifndef clox_compiler_h
#define clox_compiler_h

#include "vm.h"
#include "object.h"

// Given source code, it compiles it by writing bytes into the chunk.
bool compile(const char *source, Chunk *Chunk);

// ? Parser
typedef struct
{
    Token current;
    Token previous;
    bool hadError;  // Flag to alert an error.
    bool panicMode; // Flag to enter in panic mode and re-sync the parser with the code.
} Parser;
// ?

// ? Lowest to highest precedence.
typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_TERNARY,
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
// ?

typedef void (*ParseFn)(bool canAssing);

// ? Parse rules
typedef struct
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;
// ?

// ? Other



#endif