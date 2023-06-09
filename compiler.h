#ifndef PEACHCOMPILER_H
#define PEACHCOMPILER_H

#include <stdio.h>
#include <stdbool.h>

struct pos
{
    int line;
    int col;
    const char* filename;
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

    union
    {
        char cval;
        const char *sval;
        unsigned int inum;
        unsigned long lnum;
        unsigned long long llnum;
        void* any;
    };
    
    // True if there is a space between this token and the next token.
    bool whitespace;

    // Points to the first character in the bracket.
    // If not between brackets, this is NULL.
    const char *between_brackets;
};

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