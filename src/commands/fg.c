#include "../headers.h"
#include "../utils/child_process_list.h"
#include "../utils/parser.h"
#include "fg.h"

extern process *PROCS_HEAD, *PROCS_TAIL;
extern long long int NUM_JOBS;

int inbuilt_fg(command *cmd)
{
    if (cmd->length != 2)
    {
        fprintf(stderr, "fg: expected exactly one argument (job number)\n");
        return -1;
    }

    long long job_number = 0;
    char *ch = NULL;
    job_number = strtol(cmd->data[1], &ch, 10);
    if (job_number <= 0)
    {
        fprintf(stderr, "fg: job number should be a positive number\n");
        return -1;
    }
    else if (job_number > NUM_JOBS)
    {
        fprintf(stderr, "fg: no such job exists\n");
        return -1;
    }

    process *p = PROCS_HEAD;

    while (p != NULL)
    {
        if (p->job_number == job_number)
        {
            break;
        }
        p = p->next;
    }

    if (p == NULL)
    {
        fprintf(stderr, "fg: no such job exists\n");
        return -1;
    }

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    if (tcsetpgrp(STDIN_FILENO, p->pid) != 0)
    {
        perror("fg");
        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        return -1;
    }
    if (kill(p->pid, SIGCONT) < 0)
    {
        perror("fg");
        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        return -1;
    }

    if (PROCS_HEAD == PROCS_TAIL && p == PROCS_HEAD)
    {
        PROCS_HEAD = NULL;
        PROCS_TAIL = NULL;
    }
    else if (p == PROCS_HEAD)
    {
        PROCS_HEAD = p->next;
        PROCS_HEAD->prev = NULL;
    }
    else if (p == PROCS_TAIL)
    {
        PROCS_TAIL = p->prev;
        PROCS_TAIL->next = NULL;
    }
    else
    {
        p->prev->next = p->next;
        p->next->prev = p->prev;
    }

    int child_status;
    waitpid(p->pid, &child_status, WUNTRACED);
    tcsetpgrp(STDIN_FILENO, getpgrp());
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    if (WIFSTOPPED(child_status))
    {
        add_to_jobs_list(p->pid, parse_command(p->command));
    }

    free(p->command);
    free(p);

    return 0;
}
