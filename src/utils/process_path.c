#include "../headers.h"
#include "process_path.h"

extern char *HOME_DIR;

char* process_path(char* raw_path)
{
    char *processed_path = calloc(PATH_MAX, sizeof(char));
    if (processed_path == NULL)
    {
        perror("calloc");
        exit(1);
    }

    if (raw_path[0] == '~' && (strlen(raw_path) == 1 || raw_path[1] == '/'))
    {
        strcpy(processed_path, HOME_DIR);
        strcat(processed_path, raw_path+1);
    }
    else
    {
        strcpy(processed_path, raw_path);
    }

    int processed_len = strlen(processed_path);
    if (processed_len > 1 && processed_path[processed_len-1] == '/')
    {
        processed_path[processed_len-1] = '\0';
    }

    return processed_path;
}
