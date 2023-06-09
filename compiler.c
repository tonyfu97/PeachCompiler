#include "compiler.h"

int compile_file(const char *filename, const char *out_filename, int flags)
{
    struct compile_process* process = compile_process_create(filename, out_filename, flags);

    if (!process)
    {
        return COMPILER_FILE_COMPILE_FAILED;
    }

    // Perform lexical analysis


    // Perform parsing


    // Perform code generation

    return 0;
}