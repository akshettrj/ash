#include "../headers.h"
#include "history.h"

void initialize_history_file()
{
    extern char *HISTORY_FILE;

    HISTORY_FILE = (char*)calloc(PATH_MAX, sizeof(char));

    sprintf(HISTORY_FILE, "%s/.2019114001_ash_history", getenv("HOME"));

    FILE *hist_file = fopen(HISTORY_FILE, "a+");

    if (hist_file == NULL)
    {
        perror("Error creating a history file");
        exit(1);
    }

    fclose(hist_file);

}

void save_to_history_file(char *commands)
{
    extern char *HISTORY_FILE;

    if (strlen(commands) == 0)
    {
        return;
    }

    char buf[ASH_MAX_COMMAND_LENGTH+1] = {0};
    memset(buf, '\0', (ASH_MAX_COMMAND_LENGTH+1)*sizeof(char));

    int history_len = 0;

    char *history[20];

    // Read from the history file
    FILE *hist_file = fopen(HISTORY_FILE, "r");

    if (hist_file == NULL)
    {
        perror("Reading History file");
        exit(1);
    }

    while (fgets(buf, sizeof(buf), hist_file))
    {
        history[history_len++] = strdup(buf);
    }

    fclose(hist_file);

    if (strcmp(history[history_len-1], commands) == 0)
    {
        for(int i=0; i<history_len; i++)
            free(history[i]);
        return ;
    }

    hist_file = fopen(HISTORY_FILE, "w");
    if (hist_file == NULL)
    {
        perror("Writing to History file");
        exit(1);
    }

    int history_commands_offset = history_len == 20 ? 1 : 0;
    for (int i=history_commands_offset; i<history_len; i++)
    {
        fputs(history[i], hist_file);
        if (i == history_len-1)    fputs("\n", hist_file);
    }
    fputs(commands, hist_file);
    fclose(hist_file);

    for(int i=0; i<history_len; i++)
        free(history[i]);
}
