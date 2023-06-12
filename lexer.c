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
    char c = lex_process->function->next_char(lex_process);

    if (lex_is_in_expression())
    {
        buffer_write(lex_process->parentheses_buffer, c);
    }

    lex_process->pos.col++;
    if (c == '\n')
    {
        lex_process->pos.line++;
        lex_process->pos.col = 1;
    }
    return c;
}

static void pushc(char c)
{
    lex_process->function->push_char(lex_process, c);
}

static char assert_next_c(char c)
{
    char next_c = nextc();
    assert(next_c == c);
    return next_c;
}

static struct pos lex_file_position()
{
    return lex_process->pos;
}

struct token *token_create(struct token *_token)
{
    memcpy(&temp_token, _token, sizeof(struct token));
    temp_token.pos = lex_file_position();
    if (lex_is_in_expression())
    {
        temp_token.between_brackets = buffer_ptr(lex_process->parentheses_buffer);
    }
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

static struct token *token_make_newline()
{
    nextc();
    return token_create(&(struct token){
        .type = TOKEN_TYPE_NEWLINE,
    });
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

struct token *token_make_one_line_comment()
{
    struct buffer *buffer = buffer_create();
    char c = 0;
    LEX_GETC_IF(buffer, c, c != '\n' && c != EOF);

    buffer_write(buffer, '\0');
    return token_create(&(struct token){
        .type = TOKEN_TYPE_COMMENT,
        .sval = buffer_ptr(buffer),
    });
}

struct token *token_make_multiline_comment()
{
    struct buffer *buffer = buffer_create();
    char c = 0;

    while (1)
    {
        LEX_GETC_IF(buffer, c, c != '*' && c != EOF);
        if (c == EOF)
        {
            compiler_error(lex_process->compiler, "Unexpected end of file");
        }
        else if (c == '*')
        {
            nextc();
            c = peekc();
            if (c == '/')
            {
                nextc();
                break;
            }
        }
    }

    buffer_write(buffer, '\0');
    return token_create(&(struct token){
        .type = TOKEN_TYPE_COMMENT,
        .sval = buffer_ptr(buffer),
    });
}

struct token *handle_comment()
{
    char c = peekc();
    if (c == '/')
    {
        nextc();
        if (peekc() == '/')
        {
            nextc();
            return token_make_one_line_comment();
        }
        else if (peekc() == '*')
        {
            nextc();
            return token_make_multiline_comment();
        }
        else
        {
            // This is a division operator.
            pushc('/');
            return token_make_operator_or_string();
        }
    }
    return NULL;
}

char lex_get_escaped_char(char c)
{
    switch (c)
    {
    case 'n':
        return '\n';
    case 't':
        return '\t';
    case '\\':
        return '\\';
    case '\'':
        return '\'';
    default:
        return c;
    }
}

void lex_pop_token()
{
    vector_pop(lex_process->tokens);
}

struct token *token_make_special_number_hexadecimal()
{
    // Skip the x
    nextc();

    struct buffer *buffer = buffer_create();
    char c = peekc();
    c = tolower(c);
    LEX_GETC_IF(buffer, c, (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
    buffer_write(buffer, '\0');
    unsigned long number = strtol(buffer_ptr(buffer), 0, 16);
    return token_make_number_for_value(number);
}

void lex_validate_binary_string(char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] != '0' && str[i] != '1')
        {
            compiler_error(lex_process->compiler, "Invalid binary string '%s'", str);
        }
    }
}

struct token *token_make_special_number_binary()
{
    // Skip the b
    nextc();

    struct buffer *buffer = buffer_create();
    char c = peekc();
    c = tolower(c);
    LEX_GETC_IF(buffer, c, (c >= '0' && c <= '1'));
    buffer_write(buffer, '\0');
    lex_validate_binary_string(buffer_ptr(buffer));
    unsigned long number = strtol(buffer_ptr(buffer), 0, 2);
    return token_make_number_for_value(number);
}

struct token *token_make_special_number()
{
    struct token* token = NULL;
    struct token *last_token = lexer_last_token();

    // In case this is actually an identifier or keyword that starts with x or b;
    if (!last_token || !(last_token->type == TOKEN_TYPE_NUMBER && last_token->llnum == 0))
    {
        return token_make_identifier_or_keyword();
    }

    lex_pop_token();  // We don't want to make a token for the first 0 in 0x1234

    char c = peekc();
    if (c == 'x')
    {
        token = token_make_special_number_hexadecimal();
    }
    else if (c == 'b')
    {
        token = token_make_special_number_binary();
    }
    // TODO: handle other special numbers

    if (!token)
    {
        compiler_error(lex_process->compiler, "Invalid special number");
    }

    return token;
}

struct token *token_make_quote()
{
    assert_next_c('\'');
    char c = nextc();
    if (c == '\\')
    {
        // In case of an escaped character in quotes like '\n'
        c = nextc();
        c = lex_get_escaped_char(c);
    }
    
    if (nextc() != '\'')
    {
        compiler_error(lex_process->compiler, "You open with a quote, but did not close with a ' character.");
    }

    return token_create(&(struct token){
        .type = TOKEN_TYPE_NUMBER,
        .cval = c,
    });
}

struct token *read_next_token()
{
    struct token *token = NULL;
    char c = peekc();

    token = handle_comment();
    if (token)
    {
        return token;
    }

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

    case 'x':
    case 'b':
        token = token_make_special_number();
        break;

    case '"':
        token = token_make_string('"', '"');
        break;

    case '\'':
        token = token_make_quote();
        break;

    case ' ':
    case '\t':
        token = handle_white_space();
        break;

    case '\n':
        token = token_make_newline();
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
