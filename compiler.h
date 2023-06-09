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

enum
{
    NUMBER_TYPE_NORMAL,
    NUMBER_TYPE_LONG,
    NUMBER_TYPE_FLOAT,
    NUMBER_TYPE_DOUBLE,
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

    struct token_number
    {
        int type;
    } num;

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

struct scope
{
    int flags;

    struct vector *entities;  // Void pointers to variables, functions, etc.
    size_t size; // Total number to bytes. Aligned to 16 bytes.

    struct scope *parent;  // NULL if no parent.
};

enum
{
    SYMBOL_TYPE_NODE,
    SYMBOL_TYPE_NATIVE_FUNCTION,
    SYMBOL_TYPE_UNKNOWN,
};

struct symbol
{
    const char *name;
    int type;
    void *data;
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

    struct vector *tokens;
    struct vector *node_vec;  // tempary vector for nodes used to construct the node tree
    struct vector *node_tree_vec;  // the actual node tree

    FILE *ofile;

    struct 
    {
        struct scope *root;
        struct scope *current;
    } scope;

    struct
    {
        struct vector *table;  // current active symbol table
        struct vector *tables;
    } symbols;
};

enum
{
    PARSE_SUCCESS,
    PARSE_FAILED,
};

enum
{
    NODE_TYPE_EXPRESSION,
    NODE_TYPE_EXPRESSION_PARENTHESES,
    NODE_TYPE_NUMBER,
    NODE_TYPE_IDENTIFIER,
    NODE_TYPE_STRING,
    NODE_TYPE_VARIABLE,
    NODE_TYPE_VARIABLE_LIST,
    NODE_TYPE_FUNCTION,
    NODE_TYPE_BODY,
    NODE_TYPE_STATEMENT_RETURN,
    NODE_TYPE_STATEMENT_IF,
    NODE_TYPE_STATEMENT_ELSE,
    NODE_TYPE_STATEMENT_WHILE,
    NODE_TYPE_STATEMENT_DO_WHILE,
    NODE_TYPE_STATEMENT_FOR,
    NODE_TYPE_STATEMENT_CONTINUE,
    NODE_TYPE_STATEMENT_BREAK,
    NODE_TYPE_STATEMENT_SWITCH,
    NODE_TYPE_STATEMENT_CASE,
    NODE_TYPE_STATEMENT_DEFAULT,
    NODE_TYPE_STATEMENT_GOTO,

    NODE_TYPE_UNARY,
    NODE_TYPE_TERNARY,
    NODE_TYPE_LABEL,

    NODE_TYPE_STRUCT,
    NODE_TYPE_UNION,
    NODE_TYPE_BRACKET,
    NODE_TYPE_CAST,
    NODE_TYPE_BLANK,
};

enum
{
    NODE_FLAG_INSIDE_EXPRESSION = 1 << 0,
};

struct node
{
    int type;
    int flags;

    struct pos pos;

    struct node_binded
    {
        struct node *owner;    // pointer to the body node
        struct node *function; // pointer to the function this node is in
    } binded;

    union
    {
        struct exp
        {
            struct node *left;
            struct node *right;
            const char *op;
        } exp;
    };

    union
    {
        char cval;
        const char *sval;
        unsigned int inum;
        unsigned long lnum;
        unsigned long long llnum;
    };
};

enum
{
    DATATYPE_FLAG_IS_SIGNED = 1 << 0,
    DATATYPE_FLAG_IS_STATIC = 1 << 1,
    DATATYPE_FLAG_IS_CONST = 1 << 2,
    DATATYPE_FLAG_IS_POINTER = 1 << 3,
    DATATYPE_FLAG_IS_ARRAY = 1 << 4,
    DATATYPE_FLAG_IS_EXTERN = 1 << 5,
    DATATYPE_FLAG_IS_RESTRICT = 1 << 6,
    DATATYPE_FLAG_IS_IGNORE_TYPECHECK = 1 << 7,
    DATATYPE_FLAG_IS_SECONDARY = 1 << 8,
    DATATYPE_FLAG_IS_STRUCT_UNION_NO_NAME = 1 << 9,
    DATATYPE_FLAG_IS_LITERAL = 1 << 10,
};

enum
{
    DATATYPE_VOID,
    DATATYPE_CHAR,
    DATATYPE_SHORT,
    DATATYPE_INT,
    DATATYPE_LONG,
    DATATYPE_FLOAT,
    DATATYPE_DOUBLE,
    DATATYPE_STRUCT,
    DATATYPE_UNION,
    DATATYPE_UNKNOWN,
};

struct datatype
{
    int flags;
    int type;
    struct datatype* datatype_secondary;  // e.g., long int. int is the secondary datatype
    const char *type_str;
    size_t size;  // number of bytes
    int pointer_depth; // e.g., **ptr = depth of 2

    union 
    {
        struct node *struct_node;
        struct node *union_node;
    };
};

enum
{
    DATATYPE_EXPECT_PRIMITIVE,
    DATATYPE_EXPECT_UNION,
    DATATYPE_EXPECT_STRUCT,
};

enum
{
    DATA_SIZE_ZERO = 0,
    DATA_SIZE_BYTE = 1,
    DATA_SIZE_WORD = 2,
    DATA_SIZE_DWORD = 4,
    DATA_SIZE_DDWORD = 8,
};

// cpprocess.c
struct compile_process *compile_process_create(const char *filename, const char *out_filename, int flags);
char compile_process_next_char(struct lex_process *lex_process);
char compile_process_peek_char(struct lex_process *lex_process);
void compile_process_push_char(struct lex_process *lex_process, char c);

// compiler.c
void compiler_error(struct compile_process *compiler, const char *message, ...);
void compiler_warning(struct compile_process *compiler, const char *message, ...);
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
bool keyword_is_datatype(const char *str);
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
bool token_is_symbol(struct token *token, char symbol);
bool token_is_newline_or_comment(struct token *token);
bool token_is_operator(struct token *token, const char *op);
bool token_is_primitive_keyword(struct token *token);

// parser.c
struct history *history_begin(int flags);
struct history *history_down(struct history *history, int flags);
void parse_single_token_to_node();
void parse_expressionable_for_op(struct history *history, const char *op);
void parse_exp_normal(struct history *history);
int parse_exp(struct history *history);
int parse_expressionable_single(struct history *history);
void parse_expressionable(struct history *history);
int parse_next();
int parse(struct compile_process *process);
void parser_datatype_adjust_size_for_secondary(struct datatype *datatype, struct token *secondary_datatype_token);
void parser_ignore_int(struct datatype *dtype);

// node.c
void node_set_vector(struct vector *vec, struct vector *root);
void node_push(struct node *node);
struct node *node_peek_or_null();
struct node *node_peek();
struct node *node_pop();
struct node *node_create(struct node *_node);
void make_exp_node(struct node *left_node, struct node *right_node, const char *op);
bool node_is_expressionable(struct node *node);
struct node *node_peek_expressionable_or_null();

// expressionable.c
#define TOTAL_OPERATOR_GROUPS 14
#define MAX_OPERATOR_IN_GROUP 12
enum
{
    ASSOCIATIVITY_LEFT_TO_RIGHT,
    ASSOCIATIVITY_RIGHT_TO_LEFT,
};
struct expressionable_op_precedence_group
{
    char *operators[MAX_OPERATOR_IN_GROUP];
    int associtivity;
};

// datatype.c
bool datatype_is_struct_or_union_for_name(const char *name);

// scope.c
struct scope *scope_alloc();
void scope_dealloc(struct scope *scope);
struct scope *scope_create_root(struct compile_process *process);
void scope_free_root(struct compile_process *process);
struct scope *scope_new(struct compile_process *process, int flags);
void scope_iteration_start(struct scope *scope);
void scope_iteration_end(struct scope *scope);
void *scope_iterate_back(struct scope *scope);
void *scope_last_entity_at_scope(struct scope *scope);
void *scope_last_entity_from_scope_stop_at(struct scope *scope, struct scope *stop_scope);
void *scope_last_entity_stop_at(struct compile_process *process, struct scope *stop_scope);
void scope_push(struct compile_process *process, void *entity, size_t entity_size);
void scope_finish(struct compile_process *process);
struct scope *scope_current(struct compile_process *process);
struct scope *scope_root(struct compile_process *process);

#endif // PEACHCOMPILER_H
