#ifndef PEACHCOMPILER_H
#define PEACHCOMPILER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "helpers/vector.h"

#define S_EQ(str, str2) \
    (str && str2 && (strcmp(str, str2) == 0))

struct pos
{
    int line;
    int col;
    const char *filename;
};

#define NUMERIC_CASE \
    case '0':        \
    case '1':        \
    case '2':        \
    case '3':        \
    case '4':        \
    case '5':        \
    case '6':        \
    case '7':        \
    case '8':        \
    case '9'

#define OPERATOR_CASE_EXCLUDE_DIVIDION \
    case '+':                          \
    case '-':                          \
    case '*':                          \
    case '>':                          \
    case '<':                          \
    case '^':                          \
    case '%':                          \
    case '!':                          \
    case '=':                          \
    case '~':                          \
    case '|':                          \
    case '&':                          \
    case '(':                          \
    case '[':                          \
    case '.':                          \
    case ',':                          \
    case '?'

#define SYMBOL_CASE \
    case '{':       \
    case '}':       \
    case ':':       \
    case ';':       \
    case '#':       \
    case '\\':      \
    case ')':       \
    case ']'

enum
{
    LEX_SUCCESS,
    LEX_FAILED,
};

enum
{
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_KEYWORD,
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_SYMBOL,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_COMMENT,
    TOKEN_TYPE_NEWLINE,
};

struct token
{
    int type;
    int flags;
    struct pos pos;

    union
    {
        char cval;
        const char *sval;
        unsigned int inum;
        unsigned long lnum;
        unsigned long long llnum;
        void *any;
    };

    // True if there is a space between this token and the next token.
    bool whitespace;

    // Points to the first character in the bracket.
    // If not between brackets, this is NULL.
    const char *between_brackets;
};

struct lex_process;
typedef char (*LEX_PROCESS_NEXT_CHAR)(struct lex_process *process);
typedef char (*LEX_PROCESS_PEEK_CHAR)(struct lex_process *process);
typedef void (*LEX_PROCESS_PUSH_CHAR)(struct lex_process *process, char c);

struct lex_process_functions
{
    LEX_PROCESS_NEXT_CHAR next_char;
    LEX_PROCESS_PEEK_CHAR peek_char;
    LEX_PROCESS_PUSH_CHAR push_char;
};

struct lex_process
{
    struct pos pos;
    struct vector *tokens;
    struct compile_process *compiler;

    int current_expression_count;
    struct buffer *parentheses_buffer;
    struct lex_process_functions *function;

    // Private data that the lexer does not understand but the user does.
    void *private;
};

enum
{
    COMPILER_FILE_COMPILE_SUCCESS = 0,
    COMPILER_FILE_COMPILE_FAILED = 1,
};

struct compile_process
{
    int flags;

    struct pos pos;
    struct compiler_process_input_file
    {
        FILE *fp;
        const char *abs_path;
    } cfile;

    FILE *ofile;
};

// cpprocess.c
struct compile_process *compile_process_create(const char *filename, const char *out_filename, int flags);
char compile_process_next_char(struct lex_process *lex_process);
char compile_process_peek_char(struct lex_process *lex_process);
void compile_process_push_char(struct lex_process *lex_process, char c);

// compiler.c
void compiler_error(struct compile_process *compiler, const char *message, ...);
int compile_file(const char *filename, const char *out_filename, int flags);

// lex_process.c
struct lex_process *lex_process_create(struct compile_process *compiler, struct lex_process_functions *functions, void *private);
void lex_process_free(struct lex_process *process);
void *lex_process_private(struct lex_process *process);
struct vector *lex_process_tokens(struct lex_process *process);

// lexer.c
int lex(struct lex_process *process);
struct token *token_create(struct token *_token);
const char *read_number_str();
unsigned long long read_number();
struct token *token_make_number_for_value(unsigned long number);
struct token *token_make_number();
struct token *read_next_token();
const char *read_op();
bool lex_is_in_expression();
struct token *token_read_special_token();
bool is_keyword(const char *str);
struct token *token_make_one_line_comment();
struct token *token_make_multiline_comment();
struct token *handle_comment();
struct token *token_make_quote();
char lex_get_escaped_char(char c);
void lex_pop_token();
struct token *token_make_special_number_hexadecimal();
void lex_validate_binary_string(char *str);
struct token *token_make_special_number_binary();
struct token *token_make_special_number();
struct lex_process *token_build_for_string(struct compile_process *compiler, const char *str);

// token.c
bool token_is_keyword(struct token *token, const char *keyword);

#endif // PEACHCOMPILER_H
