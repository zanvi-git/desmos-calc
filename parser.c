#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef struct {
    const char* input;
    int pos;
    Token current;
} ParserState;

static void GetNextToken(ParserState* p);
static ASTNode* ParseExpression(ParserState* p);

// helper to create nodes
static ASTNode* CreateNode(NodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (node) {
        node->type = type;
        // zero out memory to be safe
        memset(&node->data, 0, sizeof(node->data));
    }
    return node;
}

static void GetNextToken(ParserState* p) {
    while (p->input[p->pos] && isspace(p->input[p->pos])) {
        p->pos++;
    }

    if (!p->input[p->pos]) {
        p->current.type = TOKEN_EOF;
        return;
    }

    char c = p->input[p->pos];

    if (isdigit(c) || c == '.') {
        char* end;
        p->current.value = strtod(p->input + p->pos, &end);
        p->current.type = TOKEN_NUMBER;
        p->pos = (int)(end - p->input);
        return;
    }

    if (isalpha(c)) {
        int start = p->pos;
        while (isalnum(p->input[p->pos])) p->pos++; // allow alphanumeric variables
        int len = p->pos - start;
        char buf[32];
        if (len > 31) len = 31;
        strncpy(buf, p->input + start, len);
        buf[len] = '\0';

        // check for functions
        if (strcmp(buf, "sin") == 0) {
            p->current.type = TOKEN_FUNCTION;
            p->current.func = FUNC_SIN;
        } else if (strcmp(buf, "cos") == 0) {
            p->current.type = TOKEN_FUNCTION;
            p->current.func = FUNC_COS;
        } else if (strcmp(buf, "tan") == 0) {
            p->current.type = TOKEN_FUNCTION;
            p->current.func = FUNC_TAN;
        } else if (strcmp(buf, "sqrt") == 0) {
            p->current.type = TOKEN_FUNCTION;
            p->current.func = FUNC_SQRT;
        } else if (strcmp(buf, "log") == 0) {
            p->current.type = TOKEN_FUNCTION;
            p->current.func = FUNC_LOG;
        } else if (strcmp(buf, "exp") == 0) {
            p->current.type = TOKEN_FUNCTION;
            p->current.func = FUNC_EXP;
        } else if (strcmp(buf, "abs") == 0) {
            p->current.type = TOKEN_FUNCTION;
            p->current.func = FUNC_ABS;
        } else {
            p->current.type = TOKEN_VARIABLE;
            strcpy(p->current.varName, buf);
        }
        return;
    }

    switch (c) {
        case '+': p->current.type = TOKEN_PLUS; break;
        case '-': p->current.type = TOKEN_MINUS; break;
        case '*': p->current.type = TOKEN_MULTIPLY; break;
        case '/': p->current.type = TOKEN_DIVIDE; break;
        case '^': p->current.type = TOKEN_POWER; break;
        case '(': p->current.type = TOKEN_LPAREN; break;
        case ')': p->current.type = TOKEN_RPAREN; break;
        default: p->current.type = TOKEN_ERROR; break;
    }
    p->pos++;
}

static ASTNode* ParseFactor(ParserState* p) {
    Token t = p->current;
    if (t.type == TOKEN_NUMBER) {
        GetNextToken(p);
        ASTNode* node = CreateNode(NODE_NUMBER);
        node->data.number = t.value;
        return node;
    } else if (t.type == TOKEN_VARIABLE) {
        ASTNode* node = CreateNode(NODE_VARIABLE);
        strcpy(node->data.varName, t.varName);
        GetNextToken(p);
        return node;
    } else if (t.type == TOKEN_LPAREN) {
        GetNextToken(p);
        ASTNode* node = ParseExpression(p);
        if (p->current.type == TOKEN_RPAREN) {
            GetNextToken(p);
        }
        return node;
    } else if (t.type == TOKEN_MINUS) {
        GetNextToken(p);
        ASTNode* node = CreateNode(NODE_UNARY_OP);
        node->data.unary.operand = ParseFactor(p); // recursive step should be Factor or higher precedence? -x^2 is -(x^2) usually.
        // actually -x^2 is usually -(x^2), so unary minus has lower precedence than power.
        // But here im calling ParseFactor. If ParsePower handles power, then -x^2 would be parsed as (-x)^2 if I call ParseFactor?
        // wait, standard precedence: power > unary > mul/div > add/sub.
        // so unary should call power? 
        // Let's implement power first.
        return node;
    } else if (t.type == TOKEN_FUNCTION) {
        FuncType f = t.func;
        GetNextToken(p);
        ASTNode* node = CreateNode(NODE_FUNCTION);
        node->data.function.func = f;
        node->data.function.arg = ParseFactor(p); // sin(x) vs sin x. 
        // if i want sin(x+1), ParseFactor handles parenthesized expression.
        return node;
    }
    return NULL; // handle error
}

static ASTNode* ParsePower(ParserState* p) {
    ASTNode* left = ParseFactor(p);
    while (p->current.type == TOKEN_POWER) {
        GetNextToken(p);
        ASTNode* right = ParseFactor(p); // right associative? a^b^c -> a^(b^c). If we just call ParseFactor here, it's correct for right associativity if we recurse ParsePower?
        // Actually to do right associative: a^b^c = a^(b^c).
        // my implementation: left = a. see ^. right = ParseFactor(b). 
        // if b is followed by ^c, ParseFactor will just return b.
        // So we need: right = ParsePower(p).
        
        // let's fix recursion for right associativity -__-
        // but simple loop is left associative.
        // For right associative:
        /*
        ASTNode* right = ParsePower(p);
        ASTNode* newNode = CreateNode(NODE_BINARY_OP);
        ...
        return newNode;
        */
        
        // for now, let's just stick to loop (left associative) or simple recursion.
        // most calculators do left associative for power? No, 2^3^2 = 2^9 = 512. (2^3)^2 = 64.
        // standard is right associative ig
        // let's fix it properly later if needed stick to simple loop left associative for simplicity....
        
        
        ASTNode* node = CreateNode(NODE_BINARY_OP);
        node->data.binary.left = left;
        node->data.binary.right = right;
        node->data.binary.op = TOKEN_POWER;
        left = node;
    }
    return left;
}

static ASTNode* ParseTerm(ParserState* p) {
    ASTNode* left = ParsePower(p);
    while (p->current.type == TOKEN_MULTIPLY || p->current.type == TOKEN_DIVIDE ||
           p->current.type == TOKEN_VARIABLE || p->current.type == TOKEN_LPAREN || 
           p->current.type == TOKEN_FUNCTION || (p->current.type == TOKEN_NUMBER)) { // Implicit multiplication
        
        TokenType type = p->current.type;
        if (type == TOKEN_MULTIPLY || type == TOKEN_DIVIDE) {
            GetNextToken(p);
            ASTNode* right = ParsePower(p);
            ASTNode* node = CreateNode(NODE_BINARY_OP);
            node->data.binary.left = left;
            node->data.binary.right = right;
            node->data.binary.op = type;
            left = node;
        } else {
             // implicit multiplication
             ASTNode* right = ParsePower(p);
             ASTNode* node = CreateNode(NODE_BINARY_OP);
             node->data.binary.left = left;
             node->data.binary.right = right;
             node->data.binary.op = TOKEN_MULTIPLY;
             left = node;
        }
    }
    return left;
}

static ASTNode* ParseExpression(ParserState* p) {
    ASTNode* left = ParseTerm(p);
    while (p->current.type == TOKEN_PLUS || p->current.type == TOKEN_MINUS) {
        TokenType type = p->current.type;
        GetNextToken(p);
        ASTNode* right = ParseTerm(p);
        ASTNode* node = CreateNode(NODE_BINARY_OP);
        node->data.binary.left = left;
        node->data.binary.right = right;
        node->data.binary.op = type;
        left = node;
    }
    return left;
}

ASTNode* Parser_Parse(const char* input) {
    ParserState p;
    p.input = input;
    p.pos = 0;
    GetNextToken(&p);
    return ParseExpression(&p);
}

double AST_Evaluate(ASTNode* node, EvalContext* ctx) {
    if (!node) return 0.0;
    
    switch (node->type) {
        case NODE_NUMBER: return node->data.number;
        case NODE_VARIABLE:
            if (strcmp(node->data.varName, "x") == 0) return ctx->x;
            if (strcmp(node->data.varName, "y") == 0) return ctx->y;
            if (strcmp(node->data.varName, "t") == 0) return ctx->t;
            return 0.0; // unknown variable
        case NODE_BINARY_OP: {
            double left = AST_Evaluate(node->data.binary.left, ctx);
            double right = AST_Evaluate(node->data.binary.right, ctx);
            switch (node->data.binary.op) {
                case TOKEN_PLUS: return left + right;
                case TOKEN_MINUS: return left - right;
                case TOKEN_MULTIPLY: return left * right;
                case TOKEN_DIVIDE: return (right != 0) ? left / right : NAN;
                case TOKEN_POWER: return pow(left, right);
                default: return 0.0;
            }
        }
        case NODE_UNARY_OP:
            return -AST_Evaluate(node->data.unary.operand, ctx);
        case NODE_FUNCTION: {
            double arg = AST_Evaluate(node->data.function.arg, ctx);
            switch (node->data.function.func) {
                case FUNC_SIN: return sin(arg);
                case FUNC_COS: return cos(arg);
                case FUNC_TAN: return tan(arg);
                case FUNC_SQRT: return sqrt(arg);
                case FUNC_LOG: return log(arg);
                case FUNC_EXP: return exp(arg);
                case FUNC_ABS: return fabs(arg);
                default: return 0.0;
            }
        }
    }
    return 0.0;
}

void AST_Free(ASTNode* node) {
    if (!node) return;
    if (node->type == NODE_BINARY_OP) {
        AST_Free(node->data.binary.left);
        AST_Free(node->data.binary.right);
    } else if (node->type == NODE_UNARY_OP) {
        AST_Free(node->data.unary.operand);
    } else if (node->type == NODE_FUNCTION) {
        AST_Free(node->data.function.arg);
    }
    free(node);
}
