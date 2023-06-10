#include <string.h>

#include "compiler.h"
#include "helpers/vector.h"
#include "helpers/buffer.h"

#define LEX_GETC_IF(buffer, c, exp) \
    for (c=peekc(); exp; c=peekc()) \
    { \
        buffer_write(buffer, c); \
        nextc(); \
    }

static struct lex_process *lex_process;
static struct token temp_token;

static char peekc()
{
    return lex_process->function->peek_char(lex_process);
}

static char nextc()
{
    return lex_process->function->next_char(lex_process);
}

static void pushc(char c)
{
    lex_process->function->push_char(lex_process, c);
}

static struct pos lex_file_position()
{
    return lex_process->pos;
}

struct token *token_create(struct token *_token)
{
    memcpy(&temp_token, _token, sizeof(struct token));
    temp_token.pos = lex_file_position();
    return &temp_token;
}

static struct token *lexer_last_token()
{
    return vector_back_or_null(lex_process->tokens);
}

static struct token *handle_white_space()
{
    struct token *last_token = lexer_last_token();
    if (last_token)
    {
        last_token->whitespace = true;
    }
    nextc();
    return read_next_token();
}

const char *read_number_str(void)
{
    // const char *num = NULL;
    struct buffer *buffer = buffer_create();
    char c = peekc();
    LEX_GETC_IF(buffer, c, c >= '0' && c <= '9');

    buffer_write(buffer, '\0');
    return buffer_ptr(buffer);
}

unsigned long long read_number()
{
    const char *s = read_number_str();
    return atoll(s);
}

struct token *token_make_number_for_value(unsigned long number)
{
    return token_create(&(struct token)
    {
        .type = TOKEN_TYPE_NUMBER,
        .llnum = number,
    });
}

struct token *token_make_number()
{
    return token_make_number_for_value(read_number());
}

struct token *read_next_token()
{
    struct token *token = NULL;
    char c = peekc();

    switch(c)
    {
        NUMERIC_CASE:
            token = token_make_number();
            break;
        
        case ' ':
        case '\t':
            token = handle_white_space();
            break;

        case EOF:
            return NULL;
        default:
            compiler_error(lex_process->compiler, "Unexpected character '%c'", c);
    }

    return token;
}

int lex(struct lex_process *process)
{
    process->current_expression_count = 0;
    process->parentheses_buffer = NULL;

    lex_process = process;
    process->pos.filename = process->compiler->cfile.abs_path;

    struct token *token = read_next_token();
    while(token)
    {
        vector_push(process->tokens, token);
        token = read_next_token();
    }

    return 0;
}
