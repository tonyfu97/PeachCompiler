#include "compiler.h"

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
