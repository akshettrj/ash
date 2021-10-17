#include "../headers.h"
#include "echo.h"

int inbuilt_echo(command *cmd)
{
    for (int i=1; i<cmd->length; i++)
    {
        printf("%s ", cmd->data[i]);
    }

    printf("\b\n");

    return 0;
}
