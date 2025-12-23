#ifndef PARSER_H
#define PARSER_H

typedef enum {
    TOKEN_NUMBER,
    TOKEN_VARIABLE, // 'x'
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_POWER,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_FUNCTION,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef enum {
    FUNC_SIN,
    FUNC_COS,
    FUNC_TAN,
    FUNC_SQRT,
    FUNC_LOG,
    FUNC_EXP,
    FUNC_ABS,
    FUNC_NONE
} FuncType;

typedef struct {
    TokenType type;
    double value;
    FuncType func;
} Token;

typedef struct {
    const char* input;
    int pos;
    Token current;
} Parser;

void Parser_Init(Parser* p, const char* input);
double Parser_Evaluate(Parser* p, double x);

#endif // PARSER_H
