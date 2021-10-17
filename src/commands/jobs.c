#include "../headers.h"
#include "jobs.h"

int inbuilt_jobs(command *cmd)
{
    int show_running_jobs = 0;
    int show_stopped_jobs = 0;

    extern process *PROCS_HEAD, *PROCS_TAIL;
    extern long long int NUM_JOBS;

    if (cmd->length > 2)
    {
        fprintf(stderr, "jobs: expected none or exactly one argument\n");
        return -1;
    }
    else if (cmd->length == 1)
    {
        show_running_jobs = 1;
        show_stopped_jobs = 1;
    }
    else if (cmd->length == 2)
    {
        if (strcmp(cmd->data[1], "-r") == 0)
        {
            show_running_jobs = 1;
            show_stopped_jobs = 0;
        }
        else if (strcmp(cmd->data[1], "-s") == 0)
        {
            show_running_jobs = 0;
            show_stopped_jobs = 1;
        }
        else
        {
            fprintf(stderr, "jobs: invalid arguments, expected one out of -r and -s\n");
            return -1;
        }
    }

    process *p = PROCS_HEAD;
    int count_printed = 0;

    while (p != NULL)
    {
        char process_state = '\0';

        char *proc_stat_file_path = calloc(PATH_MAX, sizeof(char));
        if (proc_stat_file_path == NULL)
        {
            printf("jobs");
            return -1;
        }
        sprintf(proc_stat_file_path, "/proc/%d/stat", p->pid);

        FILE *proc_stat_file = fopen(proc_stat_file_path, "r");
        if (proc_stat_file == NULL)
        {
            fprintf(stderr, "jobs: could not open the stat file for process with pid %d\n", p->pid);
            free(proc_stat_file_path);
            return -1;
        }

        char *read_buffer = calloc(ASH_MAX_COMMAND_LENGTH, sizeof(char));
        if (read_buffer == NULL)
        {
            perror("jobs");
            free(proc_stat_file_path);
            fclose(proc_stat_file);
            return -1;
        }

        int values_read = 0;
        while (values_read < 8)
        {
            fscanf(proc_stat_file, "%s", read_buffer);
            values_read += 1;
            if (values_read == 3)
            {
                process_state = read_buffer[0];
            }
        }

        int is_running = ((process_state == 'R' || process_state == 'S') ? 1 : 0);

        if (show_running_jobs == 0 && is_running == 1) p = p->next;
        else if (show_stopped_jobs == 0 && is_running == 0) p = p->next;
        else {
            count_printed += 1;
            printf( \
                    "[%lld] %s %s [%d]\n", \
                    p->job_number, \
                    is_running == 1 ? "Running" : "Stopped", \
                    p->command, \
                    p->pid \
                  );
            p = p->next;
        }
        // p->is_running ? "Running" : "Stopped",
    }

    if (count_printed == 0)
    {
        printf("No jobs found.\n");
    }

    return 0;
}
