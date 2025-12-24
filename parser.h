#ifndef PARSER_H
#define PARSER_H

typedef enum {
    TOKEN_NUMBER,
    TOKEN_VARIABLE, 
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
    char varName[32]; // For variables
} Token;

typedef enum {
    NODE_NUMBER,
    NODE_VARIABLE,
    NODE_BINARY_OP,
    NODE_UNARY_OP, 
    NODE_FUNCTION
} NodeType;

typedef struct ASTNode ASTNode;

struct ASTNode {
    NodeType type;
    union {
        double number;
        char varName[32];
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            TokenType op;
        } binary;
        struct {
            struct ASTNode* operand;
        } unary; // MINUS
        struct {
            struct ASTNode* arg;
            FuncType func;
        } function;
    } data;
};

// context includes x, y, t, and other variables 
typedef struct {
     double x;
     double y;
     double t;
} EvalContext;

ASTNode* Parser_Parse(const char* input);
double AST_Evaluate(ASTNode* node, EvalContext* ctx);
void AST_Free(ASTNode* node);

#endif
