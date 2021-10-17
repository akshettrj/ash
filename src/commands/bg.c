#include "../headers.h"
#include "bg.h"

extern process *PROCS_HEAD, *PROCS_TAIL;
extern long long int NUM_JOBS;

int inbuilt_bg(command *cmd)
{
    if (cmd->length != 2)
    {
        fprintf(stderr, "bg: expected exactly one argument (job number)\n");
        return -1;
    }

    long long job_number = 0;
    char *ch = NULL;
    job_number = strtol(cmd->data[1], &ch, 10);
    if (job_number <= 0)
    {
        fprintf(stderr, "bg: job number should be a positive number\n");
        return -1;
    }
    else if (job_number > NUM_JOBS)
    {
        fprintf(stderr, "bg: no such job exists\n");
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
        fprintf(stderr, "bg: no such job exists\n");
        return -1;
    }

    if (kill(p->pid, SIGCONT) != 0)
    {
        if (errno == EINVAL)
            fprintf(stderr, "bg: the specified signal number is invalid\n");

        else if (errno == EPERM)
            fprintf(stderr, "bg: may not have permissions to send signal to the process with pid %d\n", p->pid);

        else if (errno == ESRCH)
            fprintf(stderr, "bg: the process with pid %d either does not exist or has terminated\n", p->pid);

        return -1;
    }

    printf("Successfully sent signal to [%lld] %s for Running in background\n", p->job_number, p->command);

    return 0;
}
