#include "compiler.h"
#include "helpers/vector.h"


static void symresolver_push_symbol(struct compile_process *process, struct symbol *sym)
{
    vector_push(process->symbols.table, &sym);
}

void sysrosolver_init(struct compile_process *process)
{
    process->symbols.tables = vector_create(sizeof(struct vector *));

}

void symresolver_new_table(struct compile_process *process)
{
    // Save the current table
    vector_push(process->symbols.tables, &process->symbols.table);

    // Overwrite the current table with a new one
    process->symbols.table = vector_create(sizeof(struct symbol *));
}

void symresolver_end_table(struct compile_process *process)
{
    struct vector *last_table = vector_back_ptr(process->symbols.tables);
    process->symbols.table = last_table;
    vector_pop(process->symbols.tables);
}

struct symbol *symresolver_get_symbol(struct compile_process *process, const char *name)
{
    vector_set_peek_pointer(process->symbols.table, 0);
    struct symbol *symbol = vector_peek_ptr(process->symbols.table);
    while (symbol)
    {
        if (S_EQ(symbol->name, name))
        {
            break;
        }
        symbol = vector_peek_ptr(process->symbols.table);
    }
    return symbol;
}

struct symbol *symresolver_get_symbol_for_native_function(struct compile_process *process, const char *name)
{
    struct symbol *symbol = symresolver_get_symbol(process, name);
    if (!symbol)
    {
        return NULL;
    }

    if (symbol->type != SYMBOL_TYPE_NATIVE_FUNCTION)
    {
        return NULL;
    }

    return symbol;
}

struct symbol *symresolver_register_symbol(struct compile_process *process, const char *name, int type, void *data)
{
    if (symresolver_get_symbol(process, name))
    {
        return NULL;
    }

    struct symbol *symbol = malloc(sizeof(struct symbol));
    symbol->name = name;
    symbol->type = type;
    symbol->data = data;
    symresolver_push_symbol(process, symbol);
    return symbol;
}

struct node *symresolver_node(struct symbol *symbol)
{
    if (symbol->type != SYMBOL_TYPE_NODE)
    {
        return NULL;
    }
    return symbol->data;
}

void symresolver_build_for_variable_node(struct compile_process *process, struct node *node)
{
    compiler_error(process, "variable not yet supported\n");
}

void symresolver_build_for_function_node(struct compile_process *process, struct node *node)
{
    compiler_error(process, "function not yet supported\n");
}

void symresolver_build_for_structure_node(struct compile_process *process, struct node *node)
{
    compiler_error(process, "structure not yet supported\n");
}

void symresolver_build_for_union_node(struct compile_process *process, struct node *node)
{
    compiler_error(process, "union not yet supported\n");
}

void symresolver_build_for_node(struct compile_process *process, struct node *node)
{
    switch(node->type)
    {
        case NODE_TYPE_VARIABLE:
            symresolver_build_for_variable_node(process, node);
            break;
        
        case NODE_TYPE_FUNCTION:
            symresolver_build_for_function_node(process, node);
            break;
        
        case NODE_TYPE_STRUCT:
            symresolver_build_for_structure_node(process, node);
            break;
        
        case NODE_TYPE_UNION:
            symresolver_build_for_union_node(process, node);
            break;

        // Ignore all other node types because they can't become symbols.
    }
}
