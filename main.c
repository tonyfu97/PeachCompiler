#include <stdio.h>

#include "compiler.h"

int main(void)
{
    int response = compile_file("./test.c", "./test.o", 0);

    if (response == COMPILER_FILE_COMPILE_FAILED)
    {
        printf("Failed to compile file\n");
        return 1;
    }
    else if (response == COMPILER_FILE_COMPILE_SUCCESS)
    {
        printf("Successfully compiled file\n");
    }
    else
    {
        printf("Unknown response from compiler\n");
        return 1;
    }

    return 0;
}
