#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>

#include "compiler.h"
#include "helpers/vector.h"
#include "helpers/buffer.h"

#define LEX_GETC_IF(buffer, c, exp)     \
    for (c = peekc(); exp; c = peekc()) \
    {                                   \
        buffer_write(buffer, c);        \
        nextc();                        \
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
    return token_create(&(struct token){
        .type = TOKEN_TYPE_NUMBER,
        .llnum = number,
    });
}

struct token *token_make_number()
{
    return token_make_number_for_value(read_number());
}

struct token *token_make_string(char start_delm, char end_delim)
{
    struct buffer *buffer = buffer_create();
    assert(nextc() == start_delm);
    char c = nextc();
    for (; c != end_delim && c != EOF; c = nextc())
    {
        if (c == '\\')
        {
            // need to handle this
        }
        buffer_write(buffer, c);
    }
    buffer_write(buffer, '\0');
    return token_create(&(struct token){
        .type = TOKEN_TYPE_STRING,
        .sval = buffer_ptr(buffer),
    });
}

static bool op_treated_as_one(char op)
{
    return op == '(' || op == '[' || op == ',' || op == '.' || op == '*' || op == '?';
}

static bool is_single_operator(char op)
{
    return op == '+' || op == '-' || op == '=' || op == '<' || op == '>' || op == '!' || op == '&' || op == '|' || op == '^' || op == '~' || op == '*' || op == '/' || op == '%' || op == '(' || op == '[' || op == ',' || op == '.' || op == '?';
}

static bool op_is_valid(const char *op)
{
    return S_EQ(op, "+") ||
           S_EQ(op, "-") ||
           S_EQ(op, "=") ||
           S_EQ(op, "<") ||
           S_EQ(op, ">") ||
           S_EQ(op, "!") ||
           S_EQ(op, "&") ||
           S_EQ(op, "|") ||
           S_EQ(op, "^") ||
           S_EQ(op, "~") ||
           S_EQ(op, "*") ||
           S_EQ(op, "/") ||
           S_EQ(op, "%") ||
           S_EQ(op, "(") ||
           S_EQ(op, "[") ||
           S_EQ(op, ",") ||
           S_EQ(op, ".") ||
           S_EQ(op, "?") ||
           S_EQ(op, "++") ||
           S_EQ(op, "--") ||
           S_EQ(op, "+=") ||
           S_EQ(op, "-=") ||
           S_EQ(op, "*=") ||
           S_EQ(op, "/=") ||
           S_EQ(op, "%=") ||
           S_EQ(op, "==") ||
           S_EQ(op, "!=") ||
           S_EQ(op, "<=") ||
           S_EQ(op, ">=") ||
           S_EQ(op, "&&") ||
           S_EQ(op, "||") ||
           S_EQ(op, "...") ||
           S_EQ(op, "<<") ||
           S_EQ(op, ">>");
}

void read_op_flush_back_keep_first(struct buffer *buffer)
{
    const char *data = buffer_ptr(buffer);
    int len = buffer->len;
    for (int i = len - 1; i >= 1; i--)
    {
        if (data[i] == '\0')
        {
            continue;
        }
        pushc(data[i]);
    }
}

const char *read_op()
{
    bool single_operator = true;
    char op = nextc();
    struct buffer *buffer = buffer_create();
    buffer_write(buffer, op);

    // If op can be a multi-character operator.
    if (!op_treated_as_one(op))
    {
        op = peekc();
        if (is_single_operator(op))
        {
            buffer_write(buffer, op);
            nextc();
            single_operator = false;
        }
    }

    buffer_write(buffer, '\0');
    char *ptr = buffer_ptr(buffer);
    if (!single_operator)
    {
        if (!op_is_valid(ptr))
        {
            read_op_flush_back_keep_first(buffer);
            ptr[1] = '\0';
        }
    }
    else if (!op_is_valid(ptr))
    {
        compiler_error(lex_process->compiler, "Invalid operator '%s'", ptr);
    }
    return ptr;
}

static void lex_new_expression()
{
    lex_process->current_expression_count++;
    if (lex_process->current_expression_count == 1)
    {
        lex_process->parentheses_buffer = buffer_create();
    }
}

static void lex_finish_expression()
{
    lex_process->current_expression_count--;
    if (lex_process->current_expression_count < 0)
    {
        compiler_error(lex_process->compiler, "Unexpected closing bracket");
    }
}

bool lex_is_in_expression()
{
    return lex_process->current_expression_count > 0;
}

bool is_keyword(const char *str)
{
    return S_EQ(str, "unsigned") ||
           S_EQ(str, "signed") ||
           S_EQ(str, "char") ||
           S_EQ(str, "short") ||
           S_EQ(str, "int") ||
           S_EQ(str, "long") ||
           S_EQ(str, "float") ||
           S_EQ(str, "double") ||
           S_EQ(str, "void") ||
           S_EQ(str, "struct") ||
           S_EQ(str, "union") ||
           S_EQ(str, "enum") ||
           S_EQ(str, "const") ||
           S_EQ(str, "__ignore_typecheck") ||
           S_EQ(str, "static") ||
           S_EQ(str, "typedef") ||
           S_EQ(str, "sizeof") ||
           S_EQ(str, "if") ||
           S_EQ(str, "else") ||
           S_EQ(str, "switch") ||
           S_EQ(str, "case") ||
           S_EQ(str, "default") ||
           S_EQ(str, "while") ||
           S_EQ(str, "do") ||
           S_EQ(str, "for") ||
           S_EQ(str, "goto") ||
           S_EQ(str, "continue") ||
           S_EQ(str, "break") ||
           S_EQ(str, "return") ||
           S_EQ(str, "extern") ||
           S_EQ(str, "restrict") ||
           S_EQ(str, "include");
}

static struct token *token_make_operator_or_string()
{
    char op = peekc();
    if (op == '<') // in case of #include <abc.h>
    {
        struct token *last_token = lexer_last_token();
        if (token_is_keyword(last_token, "include"))
        {
            return token_make_string('<', '>');
        }
    }

    struct token *token = token_create(&(struct token){
        .type = TOKEN_TYPE_OPERATOR,
        .sval = read_op(),
    });

    if (op == '(')
    {
        lex_new_expression();
    }
    return token;
}

static struct token *token_make_symbol()
{
    char c = nextc();
    if (c == ')')
    {
        lex_finish_expression();
    }

    struct token *token = token_create(&(struct token){
        .type = TOKEN_TYPE_SYMBOL,
        .cval = c,
    });

    return token;
}

static struct token *token_make_identifier_or_keyword()
{
    struct buffer *buffer = buffer_create();
    char c = peekc();
    LEX_GETC_IF(buffer, c, (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '1' && c <= '9') || c == '_');

    buffer_write(buffer, '\0');
    char *str = buffer_ptr(buffer);

    // Check if this is a keyword
    if (is_keyword(str))
    {
        return token_create(&(struct token){
            .type = TOKEN_TYPE_KEYWORD,
            .sval = str,
        });
    }

    return token_create(&(struct token){
        .type = TOKEN_TYPE_IDENTIFIER,
        .sval = str,
    });
}

struct token *token_read_special_token()
{
    char c = peekc();
    if (isalpha(c) || c == '_')
    {
        return token_make_identifier_or_keyword();
    }
    return NULL;
}

struct token *read_next_token()
{
    struct token *token = NULL;
    char c = peekc();

    switch (c)
    {
    NUMERIC_CASE:
        token = token_make_number();
        break;

    OPERATOR_CASE_EXCLUDE_DIVIDION:
        token = token_make_operator_or_string();
        break;

    SYMBOL_CASE:
        token = token_make_symbol();
        break;

    case '"':
        token = token_make_string('"', '"');
        break;

    case ' ':
    case '\t':
        token = handle_white_space();
        break;

    case EOF:
        return NULL;
    default:
        token = token_read_special_token();
        if (!token)
        {
            compiler_error(lex_process->compiler, "Unexpected character '%c'", c);
        }
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
    while (token)
    {
        vector_push(process->tokens, token);
        token = read_next_token();
    }

    return 0;
}
