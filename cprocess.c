#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "helpers/vector.h"

struct compile_process* compile_process_create(const char *filename, const char *out_filename, int flags)
{
    FILE* file = fopen(filename, "r");
    if (!file)
    {
        fclose(file);
        return NULL;
    }

    FILE* out_file = NULL;
    if (!out_filename)
    {
        out_file = fopen(out_filename, "w");
        if (!out_file)
        {
            fclose(file);
            return NULL;
        }
    }

    struct compile_process* process = calloc(1, sizeof(struct compile_process));
    process->node_vec = vector_create(sizeof(struct node*));
    process->node_tree_vec = vector_create(sizeof(struct node*));

    process->flags = flags;
    process->cfile.fp = file;
    process->ofile = out_file;

    return process;
}

char compile_process_next_char(struct lex_process *lex_process)
{
    struct compile_process *compiler = lex_process->compiler;
    compiler->pos.col++;
    char c = fgetc(compiler->cfile.fp);
    if (c == '\n')
    {
        compiler->pos.line++;
        compiler->pos.col = 1;
    }
    return c;
}

char compile_process_peek_char(struct lex_process *lex_process)
{
    struct compile_process *compiler = lex_process->compiler;
    char c = getc(compiler->cfile.fp);
    ungetc(c, compiler->cfile.fp);
    return c;
}

void compile_process_push_char(struct lex_process *lex_process, char c)
{
    struct compile_process *compiler = lex_process->compiler;
    ungetc(c, compiler->cfile.fp);
    // compiler->pos.col--;
}
