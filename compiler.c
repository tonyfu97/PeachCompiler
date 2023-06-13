#include <stdarg.h>
#include <stdlib.h>

#include "compiler.h"

struct lex_process_functions compiler_lex_process_functions =
{
    .next_char = compile_process_next_char,
    .peek_char = compile_process_peek_char,
    .push_char = compile_process_push_char,
};

void compiler_error(struct compile_process *compiler, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, " on line %i, column %i, in file %s\n", compiler->pos.line, compiler->pos.col, compiler->pos.filename);
    exit(-1);
}


int compile_file(const char *filename, const char *out_filename, int flags)
{
    struct compile_process* process = compile_process_create(filename, out_filename, flags);

    if (!process)
    {
        return COMPILER_FILE_COMPILE_FAILED;
    }

    // Perform lexical analysis
    struct lex_process* lex_process = lex_process_create(process, &compiler_lex_process_functions, NULL);
    if (!lex_process)
    {
        return COMPILER_FILE_COMPILE_FAILED;
    }

    if (lex(lex_process) != LEX_SUCCESS)
    {
        return COMPILER_FILE_COMPILE_FAILED;
    }

    process->tokens = lex_process->tokens;

    // Perform parsing
    if (parse(process) != PARSE_SUCCESS)
    {
        return COMPILER_FILE_COMPILE_FAILED;
    }

    // Perform code generation

    return 0;
}
