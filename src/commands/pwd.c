#include "../headers.h"
#include "pwd.h"

int inbuilt_pwd(command *cmd)
{
    if (cmd->length > 1)
    {
        fprintf(stderr, "pwd: expected no arguments\n");
        return -1;
    }

    char *curr_dir = calloc(PATH_MAX, sizeof(char));

    if (getcwd(curr_dir, PATH_MAX) == NULL)
    {
        perror("pwd");
        return -1;
    }

    printf("%s\n", curr_dir);

    free(curr_dir);

    return 0;
}
