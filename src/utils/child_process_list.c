#include "../headers.h"
#include "child_process_list.h"

process* new_process_node()
{
    process *p = NULL;
    p = (process*)calloc(1, sizeof(process));

    if (p == NULL)
    {
        perror("process node");
        exit(1);
    }

    p->pid = -1;
    p->command = NULL;
    p->job_number = -1;
    p->next = NULL;
    p->prev = NULL;

    return p;
}

void add_to_jobs_list(pid_t child_pid, command *args)
{
    extern process *PROCS_HEAD, *PROCS_TAIL;
    extern long long int NUM_JOBS;

    process *node = new_process_node();
    node->pid = child_pid;
    node->job_number = ++NUM_JOBS;
    node->next = NULL;
    node->prev = NULL;

    node->command = calloc(ASH_MAX_COMMAND_LENGTH, sizeof(char));
    if (node->command == NULL)
    {
        perror("ash");
        exit(1);
    }
    strcpy(node->command, args->data[0]);
    for (int i=1; i<(args->length-1); i++)
    {
        strcat(node->command, " ");
        strcat(node->command, args->data[i]);
    }

    // add lexicographically to the list
    if (PROCS_HEAD == NULL)
    {
        // Empty List
        PROCS_HEAD = node;
        PROCS_TAIL = node;
    }
    else
    {
        process *p = PROCS_HEAD;
        while (p != NULL)
        {
            if (strcmp(p->command, node->command) <= 0)
                p = p->next;
            else
                break;
        }

        if (p == NULL)
        {
            // Append to end of the list
            node->next = NULL;
            node->prev = PROCS_TAIL;

            PROCS_TAIL->next = node;
            PROCS_TAIL = node;
        }
        else if (p == PROCS_HEAD)
        {
            node->prev = NULL;
            node->next = PROCS_HEAD;
            PROCS_HEAD->prev = node;
            PROCS_HEAD = node;
        }
        else
        {
            // Add before p
            node->prev = p->prev;
            node->next = p;
            p->prev = node;
            if (node->prev != NULL)
            {
                node->prev->next = node;
            }
        }
    }

    fprintf(stderr, "[BG PROCESS] %d - %s\n", child_pid, args->data[0]);
}
