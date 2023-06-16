#include "compiler.h"

#define PRIMITIVE_TYPE_TOTAL 7
const char *primitive_types[PRIMITIVE_TYPE_TOTAL] = {"int", "float", "double", "char", "short", "long", "void"};

bool token_is_keyword(struct token *token, const char *keyword)
{
    return token->type == TOKEN_TYPE_KEYWORD && S_EQ(token->sval, keyword);
}

bool token_is_symbol(struct token *token, char symbol)
{
    return token->type == TOKEN_TYPE_SYMBOL && token->cval == symbol;
}

bool token_is_newline_or_comment(struct token *token)
{
    return token->type == TOKEN_TYPE_NEWLINE ||
           token->type == TOKEN_TYPE_COMMENT ||
           token_is_symbol(token, '\\');
}

bool token_is_operator(struct token *token, const char *op)
{
    return token->type == TOKEN_TYPE_OPERATOR && S_EQ(token->sval, op);
}

bool token_is_primitive_keyword(struct token *token)
{
    if (token->type != TOKEN_TYPE_KEYWORD)
    {
        return false;
    }

    for (int i = 0; i < PRIMITIVE_TYPE_TOTAL; i++)
    {
        if (S_EQ(token->sval, primitive_types[i]))
        {
            return true;
        }
    }

    return false;
}
