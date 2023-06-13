#include "compiler.h"
#include "helpers/vector.h"

static struct compile_process *current_process;
static struct token *parser_last_token;

static void parser_ignore_nl_or_comment(struct token *token)
{
    while (token && token_is_newline_or_comment(token))
    {
        vector_peek(current_process->tokens); // skip the nl or comment token
        token = vector_peek_no_increment(current_process->tokens);
    }
}

static struct token *next_token()
{
    struct token *next_token = vector_peek_no_increment(current_process->tokens);
    parser_ignore_nl_or_comment(next_token);
    current_process->pos = next_token->pos;
    parser_last_token = next_token;
    return vector_peek(current_process->tokens); // not just next_token because parser_ignore_nl_or_comment may have incremented the pointer.
}

static struct token *token_peek_next()
{
    struct token *next_token = vector_peek_no_increment(current_process->tokens);
    parser_ignore_nl_or_comment(next_token);
    return vector_peek_no_increment(current_process->tokens); // not just next_token because parser_ignore_nl_or_comment may have incremented the pointer.
}

void parse_single_token_to_node()
{
    struct token *token = next_token();
    struct node *node = NULL;
    switch (token->type)
    {
    case TOKEN_TYPE_NUMBER:
        node = node_create(&(struct node){.type = NODE_TYPE_NUMBER, .llnum = token->llnum});
        break;
    case TOKEN_TYPE_IDENTIFIER:
        node = node_create(&(struct node){.type = NODE_TYPE_IDENTIFIER, .sval = token->sval});
        break;
    case TOKEN_TYPE_STRING:
        node = node_create(&(struct node){.type = NODE_TYPE_STRING, .sval = token->sval});
        break;
    default:
        compiler_error(current_process, "Unexpected single token type %i\n", token->type);
    }
}

int parse_next()
{
    struct token *token = token_peek_next();
    if (!token)
    {
        return -1;
    }

    switch (token->type)
    {
    case TOKEN_TYPE_NUMBER:
    case TOKEN_TYPE_IDENTIFIER:
    case TOKEN_TYPE_STRING:
        parse_single_token_to_node();
        break;
    }
    return 0;
}

int parse(struct compile_process *process)
{
    current_process = process;
    parser_last_token = NULL;
    node_set_vector(process->node_vec, process->node_tree_vec);
    struct node *node = NULL;
    vector_set_peek_pointer(process->tokens, 0);

    while (parse_next() == 0)
    {
        node = node_peek();
        vector_push(process->node_tree_vec, &node);
    }
    return PARSE_SUCCESS;
}