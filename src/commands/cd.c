#include "../headers.h"
#include "cd.h"
#include "../utils/process_path.h"

int inbuilt_cd(command *cmd)
{
    extern char *HOME_DIR;
    extern char *PREV_DIR;

    if (cmd->length > 2)
    {
        fprintf(stderr, "cd: one argument (destination path) was expected\n");
        return -1;
    }

    char *new_dir =  calloc(PATH_MAX, sizeof(char));
    char *curr_dir = calloc(PATH_MAX, sizeof(char));

    if (new_dir == NULL || curr_dir == NULL)
    {
        perror("cd");
        if (new_dir != NULL)    free(new_dir);
        if (curr_dir != NULL)   free(curr_dir);
        return -1;
    }

    if (getcwd(curr_dir, PATH_MAX) == NULL)
    {
        perror("cd");
        if (new_dir != NULL)    free(new_dir);
        if (curr_dir != NULL)   free(curr_dir);
        return -1;
    }

    if (cmd->length == 1)
    {
        strcpy(new_dir, HOME_DIR);
    }
    else if (cmd->data[1][0] == '-' && strlen(cmd->data[1]) == 1)
    {
        if(strlen(PREV_DIR) == 0)
        {
            fprintf(stderr, "There is no previous directory set!\n");
            if (new_dir != NULL)    free(new_dir);
            if (curr_dir != NULL)   free(curr_dir);
            return -1;
        }
        strcpy(new_dir, PREV_DIR);
    }
    else
    {
        char *processed_path = process_path(cmd->data[1]);
        strcpy(new_dir, processed_path);
        free(processed_path);
    }

    if (chdir(new_dir) != 0)
    {
        perror(new_dir);
        if (new_dir != NULL)    free(new_dir);
        if (curr_dir != NULL)   free(curr_dir);
        return -1;
    }

    strcpy(PREV_DIR, curr_dir);

    if (new_dir != NULL)    free(new_dir);
    if (curr_dir != NULL)   free(curr_dir);

    return 0;
}
