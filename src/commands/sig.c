#include "../headers.h"
#include "sig.h"

int inbuilt_sig(command *cmd)
{
    if (cmd->length != 3)
    {
        fprintf(stderr, "sig: requires 2 arguments: job number and signal number\n");
        return -1;
    }

    extern process *PROCS_HEAD, *PROCS_TAIL;

    long long int job_number;
    int signal_number;

    {
        char *end = NULL;
        job_number = strtoll(cmd->data[1], &end, 10);
        if (*end || job_number <= 0)
        {
            fprintf(stderr, "sig: the job number should be a positive number\n");
            return -1;
        }

        end = NULL;
        signal_number = strtoq(cmd->data[2], &end, 10);
        if (*end || signal_number <= 0)
        {
            fprintf(stderr, "sig: the signal number should be a positive number\n");
            return -1;
        }
    }

    int process_id = -1;

    process *p = PROCS_HEAD;

    while (p != NULL)
    {
        if (p->job_number == job_number)
        {
            process_id = p->pid;
            break;
        }
        p = p->next;
    }

    if (p == NULL)
    {
        fprintf(stderr, "sig: no job with job number [%lld] found\n", job_number);
        return -1;
    }

    int ret = kill(process_id, signal_number);
    if (ret != 0)
    {
        if (errno == EINVAL)
            fprintf(stderr, "sig: the specified signal number is invalid\n");

        else if (errno == EPERM)
            fprintf(stderr, "sig: may not have permissions to send signal to the process with pid %d\n", process_id);

        else if (errno == ESRCH)
            fprintf(stderr, "sig: the process with pid %d either does not exist or has terminated\n", process_id);

        return -1;
    }

    printf("Successfully sent signal number %d to process with pid %d and job number [%lld]\n", signal_number, process_id, job_number);

    return 0;
}
