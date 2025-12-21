#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

static void GetNextToken(Parser* p);

void Parser_Init(Parser* p, const char* input) {
    p->input = input;
    p->pos = 0;
    GetNextToken(p);
}

static void GetNextToken(Parser* p) {
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
        while (isalpha(p->input[p->pos])) p->pos++;
        int len = p->pos - start;
        char buf[32];
        if (len > 31) len = 31;
        strncpy(buf, p->input + start, len);
        buf[len] = '\0';

        if (strcmp(buf, "x") == 0) {
            p->current.type = TOKEN_VARIABLE;
        } else if (strcmp(buf, "sin") == 0) {
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
            p->current.type = TOKEN_ERROR;
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

static double Expression(Parser* p, double x);

static double Factor(Parser* p, double x) {
    Token t = p->current;
    if (t.type == TOKEN_NUMBER) {
        GetNextToken(p);
        return t.value;
    } else if (t.type == TOKEN_VARIABLE) {
        GetNextToken(p);
        return x;
    } else if (t.type == TOKEN_LPAREN) {
        GetNextToken(p);
        double val = Expression(p, x);
        if (p->current.type == TOKEN_RPAREN) {
            GetNextToken(p);
        }
        return val;
    } else if (t.type == TOKEN_MINUS) {
        GetNextToken(p);
        return -Factor(p, x);
    } else if (t.type == TOKEN_FUNCTION) {
        FuncType f = t.func;
        GetNextToken(p);
        double val = Factor(p, x);
        switch (f) {
            case FUNC_SIN: return sin(val);
            case FUNC_COS: return cos(val);
            case FUNC_TAN: return tan(val);
            case FUNC_SQRT: return sqrt(val);
            case FUNC_LOG: return log(val);
            case FUNC_EXP: return exp(val);
            case FUNC_ABS: return fabs(val);
            default: return 0;
        }
    }
    return 0;
}

static double Power(Parser* p, double x) {
    double left = Factor(p, x);
    while (p->current.type == TOKEN_POWER) {
        GetNextToken(p);
        double right = Power(p, x); // Right associative
        left = pow(left, right);
    }
    return left;
}

static double Term(Parser* p, double x) {
    double left = Power(p, x);
    while (p->current.type == TOKEN_MULTIPLY || p->current.type == TOKEN_DIVIDE) {
        TokenType type = p->current.type;
        GetNextToken(p);
        double right = Power(p, x);
        if (type == TOKEN_MULTIPLY) left *= right;
        else if (right != 0) left /= right;
        else left = 0; // Or NaN
    }
    return left;
}

static double Expression(Parser* p, double x) {
    double left = Term(p, x);
    while (p->current.type == TOKEN_PLUS || p->current.type == TOKEN_MINUS) {
        TokenType type = p->current.type;
        GetNextToken(p);
        double right = Term(p, x);
        if (type == TOKEN_PLUS) left += right;
        else left -= right;
    }
    return left;
}

double Parser_Evaluate(Parser* p, double x) {
    return Expression(p, x);
}
