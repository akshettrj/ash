#include "../headers.h"
#include "history.h"

int inbuilt_history(command *cmd)
{
    extern char *HISTORY_FILE;
    int fetch_history_len = 10;

    if (cmd->length > 2)
    {
        fprintf(stderr, "history: more than expected arguments passed\n");
        return -1;
    }

    else if(cmd->length == 2)
    {
        char *end;
        fetch_history_len = strtoll(cmd->data[1], &end, 10);
        if (*end || fetch_history_len < 0 || fetch_history_len > 20)
        {
            fprintf(stderr, "history: expected a number between 0 and 20\n");
            return -1;
        }
    }

    if (fetch_history_len == 0)
    {
        return 0;
    }

    char *history[20];
    int hist_read = 0;

    FILE *hist_file = fopen(HISTORY_FILE, "r");
    if (hist_file == NULL)
    {
        perror(HISTORY_FILE);
        return -1;
    }

    char buf[ASH_MAX_COMMAND_LENGTH+1];
    memset(buf, '\0', sizeof(buf));

    while (fgets(buf, sizeof(buf), hist_file))
    {
        history[hist_read] = strdup(buf);
        hist_read += 1;
    }

    if (fclose(hist_file) != 0)
    {
        perror(HISTORY_FILE);
    }

    int history_print_offset = hist_read - fetch_history_len;
    if (history_print_offset < 0)
        history_print_offset = 0;


    for (int i=history_print_offset; i<hist_read; i++)
    {
        printf("%s", history[i]);
    }

    if (history_print_offset < hist_read)
        printf("\n");

    fflush(stdout);

    // Freeing Memory
    for (int i=0; i<hist_read; i++)
        free(history[i]);

    return 0;
}
