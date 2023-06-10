#include "compiler.h"

bool token_is_keyword(struct token *token, const char *keyword)
{
    return token->type == TOKEN_TYPE_KEYWORD && S_EQ(token->sval, keyword);
}
