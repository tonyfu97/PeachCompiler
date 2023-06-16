#include "compiler.h"
#include "helpers/vector.h"

#include <assert.h>

static struct compile_process *current_process;
static struct token *parser_last_token;
extern struct expressionable_op_precedence_group op_precedence[TOTAL_OPERATOR_GROUPS];

struct history
{
    int flags;
};

struct history *history_begin(int flags)
{
    struct history *history = malloc(sizeof(struct history));
    history->flags = flags;
    return history;
}

struct history *history_down(struct history *history, int flags)
{
    struct history *new_history = malloc(sizeof(struct history));
    memcpy(new_history, history, sizeof(struct history));
    return new_history;
}

static void parser_ignore_nl_or_comment(struct token *token)
{
    while (token && token_is_newline_or_comment(token))
    {
        vector_peek(current_process->tokens); // skip the nl or comment token
        token = vector_peek_no_increment(current_process->tokens);
    }
}

static struct token *token_next()
{
    struct token *next_token = vector_peek_no_increment(current_process->tokens);
    parser_ignore_nl_or_comment(next_token);
    current_process->pos = next_token->pos;
    parser_last_token = next_token;
    return vector_peek(current_process->tokens); // not just next_token because parser_ignore_nl_or_comment may have incremented the pointer.
}

// Like token_next(), but doesn't increment the pointer.
static struct token *token_peek_next()
{
    struct token *next_token = vector_peek_no_increment(current_process->tokens);
    parser_ignore_nl_or_comment(next_token);
    return vector_peek_no_increment(current_process->tokens); // not just next_token because parser_ignore_nl_or_comment may have incremented the pointer.
}

void parse_single_token_to_node()
{
    struct token *token = token_next();
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

void parse_expressionable_for_op(struct history *history, const char *op)
{
    parse_expressionable(history);
}

static int parser_get_op_precedence_for_op(const char *op, struct expressionable_op_precedence_group **group_out)
{
    *group_out = NULL;
    for (int i = 0; i < TOTAL_OPERATOR_GROUPS; i++)
    {
        struct expressionable_op_precedence_group *group = &op_precedence[i];
        for (int j = 0; group->operators[j]; j++)
        {
            const char *_op = op_precedence[i].operators[j];
            if (S_EQ(op, _op))
            {
                *group_out = group;
                return i;
            }
        }
    }

    return -1;
}

static bool parser_left_op_has_priority(const char *op_left, const char *op_right)
{
    struct expressionable_op_precedence_group *group_left = NULL;
    struct expressionable_op_precedence_group *group_right = NULL;

    if (S_EQ(op_left, op_right))
    {
        return false;
    }

    int precedence_left = parser_get_op_precedence_for_op(op_left, &group_left);
    int precedence_right = parser_get_op_precedence_for_op(op_right, &group_right);
    if (group_left->associtivity == ASSOCIATIVITY_RIGHT_TO_LEFT)
    {
        return false;
    }

    return precedence_left <= precedence_right;
}

void parser_node_shift_children_left(struct node *node)
{
    assert(node->type == NODE_TYPE_EXPRESSION);
    assert(node->exp.right->type != NODE_TYPE_EXPRESSION);

    const char *right_op = node->exp.right->exp.op;
    struct node *new_exp_left_node = node->exp.left;
    struct node *new_exp_right_node = node->exp.right->exp.left;
    make_exp_node(new_exp_left_node, new_exp_right_node, node->exp.op);
    struct node *new_left_operand = node_pop();
    struct node *new_right_operand = node->exp.right->exp.right;

    node->exp.left = new_left_operand;
    node->exp.right = new_right_operand;
    node->exp.op = right_op;
}

void parser_reorder_expression(struct node **node_out)
{
    struct node *node = *node_out;
    if (node->type != NODE_TYPE_EXPRESSION)
    {
        return;
    }

    // No expression. Nothign to reorder.
    if (node->exp.left->type == NODE_TYPE_EXPRESSION && node->exp.right && node->exp.right->type != NODE_TYPE_EXPRESSION)
    {
        return;
    }

    // E.g. 30+(50+20). 30 is not an exp, but 50+20 is. We need to reorder.
    if (node->exp.left->type != NODE_TYPE_EXPRESSION && node->exp.right && node->exp.right->type == NODE_TYPE_EXPRESSION)
    {
        const char *right_op = node->exp.right->exp.op;
        if (parser_left_op_has_priority(node->exp.op, right_op))
        {
            parser_node_shift_children_left(node);
            parser_reorder_expression(&node->exp.left);
            parser_reorder_expression(&node->exp.right);
        }
    }
}

void parse_exp_normal(struct history *history)
{
    struct token *op_token = token_peek_next();
    const char *op = op_token->sval;
    struct node *node_left = node_peek_expressionable_or_null();

    if (!node_left)
    {
        return;
    }

    // Pop off operator token
    token_next();

    // Pop off left node
    node_pop();
    node_left->flags |= NODE_FLAG_INSIDE_EXPRESSION;
    parse_expressionable_for_op(history_down(history_begin(0), history->flags), op);

    // Pop off right node
    struct node *node_right = node_pop();
    node_right->flags |= NODE_FLAG_INSIDE_EXPRESSION;
    make_exp_node(node_left, node_right, op);
    struct node *exp_node = node_pop();

    // Reorder the expression
    parser_reorder_expression(&exp_node);
    node_push(exp_node);
}

int parse_exp(struct history *history)
{
    parse_exp_normal(history);
    return 0;
}

void parse_identifier(struct history *history)
{
    assert(token_peek_next()->type == TOKEN_TYPE_IDENTIFIER);
    parse_single_token_to_node();
}

static bool is_keyword_variation_modifier(const char *val)
{
    return S_EQ(val, "const") ||
           S_EQ(val, "static") ||
           S_EQ(val, "__ignore_typecheck__") ||
           S_EQ(val, "extern") ||
           S_EQ(val, "unsigned") ||
           S_EQ(val, "signed");
}

void parse_datatype_modifiers(struct datatype *dtype)
{
    struct token *token = token_peek_next();
    while(token && is_keyword_variation_modifier(token->sval))
    {
        if (!is_keyword_variation_modifier(token->sval))
        {
            break;
        }
        
        if (S_EQ(token->sval, "const"))
        {
            dtype->flags |= DATATYPE_FLAG_IS_CONST;
        }
        else if (S_EQ(token->sval, "static"))
        {
            dtype->flags |= DATATYPE_FLAG_IS_STATIC;
        }
        else if (S_EQ(token->sval, "__ignore_typecheck__"))
        {
            dtype->flags |= DATATYPE_FLAG_IS_IGNORE_TYPECHECK;
        }
        else if (S_EQ(token->sval, "extern"))
        {
            dtype->flags |= DATATYPE_FLAG_IS_EXTERN;
        }
        else if (S_EQ(token->sval, "unsigned"))
        {
            dtype->flags &= ~DATATYPE_FLAG_IS_SIGNED;
        }
        else if (S_EQ(token->sval, "signed"))
        {
            dtype->flags |= DATATYPE_FLAG_IS_SIGNED;
        }
        token_next();
        token = token_peek_next();
    }
}

void parse_datatype(struct datatype *dtype)
{
    memset(dtype, 0, sizeof(struct datatype));
    dtype->flags |= DATATYPE_FLAG_IS_SIGNED;

    parse_datatype_modifiers(dtype);
    parse_datatype_type(dtype);
    parse_datatype_modifiers(dtype);
}

void parse_variable_function_or_struct_union(struct history *history)
{
    struct datatype dtype;
    parse_datatype(&dtype);
}

void parse_keyword(struct history *history)
{
    struct token *token = token_peek_next();
    if (is_keyword_variation_modifier(token->sval) || keyword_is_datatype(token->sval))
    {
        parse_variable_function_or_struct_union(history);
    }
}

int parse_expressionable_single(struct history *history)
{
    struct token *token = token_peek_next();
    if (!token)
    {
        return -1;
    }
    history->flags |= NODE_FLAG_INSIDE_EXPRESSION;

    int res = -1;
    switch (token->type)
    {
    case TOKEN_TYPE_NUMBER:
        parse_single_token_to_node();
        res = 0;
        break;
    case TOKEN_TYPE_IDENTIFIER:
        parse_identifier(history);
        res = 0;
        break;
    case TOKEN_TYPE_OPERATOR:
        parse_exp(history);
        res = 0;
        break;
    case TOKEN_TYPE_KEYWORD:
        parse_keyword(history);
        res = 0;
        break;
    }

    return res;
}

void parse_expressionable(struct history *history)
{
    while (parse_expressionable_single(history) == 0)
    {
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
        parse_expressionable_single(history_begin(0));
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