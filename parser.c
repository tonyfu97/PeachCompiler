#include "compiler.h"
#include "helpers/vector.h"

static struct compile_process *current_process;

int parse_next()
{
    return 0;
}

int parse(struct compile_process *process)
{
    current_process = process;

    struct node *node = NULL;
    vector_set_peek_pointer(process->tokens, 0);
    while(parse_next() == 0)
    {
        // TODO: node = node_peek;
        vector_push(process->node_tree_vec, &node);
    }
    return PARSE_SUCCESS;
}