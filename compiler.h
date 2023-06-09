#ifndef PEACHCOMPILER_H
#define PEACHCOMPILER_H

#include <stdio.h>

enum
{
    COMPILER_FILE_COMPILE_SUCCESS = 0,
    COMPILER_FILE_COMPILE_FAILED = 1,
};

struct compile_process
{
    int flags;

    struct compiler_process_input_file
    {
        FILE* fp;
        const char* abs_path;
    } cfile;

    FILE* ofile;
};

int compile_file(const char *filename, const char *out_filename, int flags);
struct compile_process* compile_process_create(const char *filename, const char *out_filename, int flags);

#endif // PEACHCOMPILER_H