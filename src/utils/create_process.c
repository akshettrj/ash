#include "../headers.h"
#include "create_process.h"

#include "process_path.h"
#include "child_process_list.h"
#include "parser.h"

pid_t pid = -1, ppid = -1;
command *global_cmd;

void pass_to_parent(int signum)
{
    if (kill(ppid, SIGTSTP) != 0)
    {
        perror("ash");
        return;
    }
}

void signal_stop_handler(int signum)
{
    if (kill(pid, SIGTSTP) != 0)
    {
        perror("ash");
        return;
    }

    add_to_jobs_list(pid, global_cmd);
}

int create_process(command *cmd)
{
    int run_in_background = 0;

    {
        char *last_arg = cmd->data[cmd->length-1];
        run_in_background = strcmp(last_arg, "&") == 0 ? 1 : 0;
    }

    command *args = new_command();

    args->length = run_in_background == 1 ? cmd->length : cmd->length+1;
    args->data = calloc(args->length, sizeof(char*));

    for (int i=0; i<args->length-1; i++)
    {
        args->data[i] = process_path(cmd->data[i]);
    }

    // Last argument must be NULL
    args->data[args->length-1] = NULL;

    ppid = getpid();
    pid = fork();

    if (pid < 0)
    {
        // Failed to fork
        perror("fork");
        for (int i=0; i<args->length-1; i++)
            free(args->data[i]);
        free(args);
        return -1;
    }
    else if (pid == 0)
    {
        // Child process
        setpgid(0, 0);

        signal(SIGTSTP, pass_to_parent);
        signal(SIGINT, SIG_DFL);

        int ret = execvp(args->data[0], args->data);
        if (ret != 0)
        {
            perror(args->data[0]);
            pid_t self_pid = getpid();
            if (kill(self_pid, SIGTERM) < 0)
            {
                kill(self_pid, SIGKILL);
            }
        }
    }
    else
    {
        // Parent Process
        pid_t child_pid = pid;
        pid_t self_pid = getpid();
        int child_status;

        if (run_in_background == 1)
        {
            add_to_jobs_list(child_pid, args);
        }
        else
        {
            global_cmd = args;
            signal(SIGTSTP, signal_stop_handler);
            signal(SIGTTIN, SIG_IGN);
            signal(SIGTTOU, SIG_IGN);
            // signal(SIGINT, signal_int_handler);
            tcsetpgrp(STDIN_FILENO, child_pid);
            waitpid(child_pid, &child_status, WUNTRACED);
            tcsetpgrp(STDIN_FILENO, getpgrp());
            signal(SIGTSTP, SIG_IGN);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            // signal(SIGINT, SIG_IGN);

            if (WIFSTOPPED(child_status))
            {
                add_to_jobs_list(child_pid, args);
            }
        }

        for (int i=0; i<args->length-1; i++)
            free(args->data[i]);
        free(args);

    }

    return 0;
}


            // process *node = new_process_node();
            // node->pid = child_pid;
            // node->job_number = ++NUM_JOBS;
            // node->next = NULL;
            // node->prev = NULL;

            // node->command = calloc(ASH_MAX_COMMAND_LENGTH, sizeof(char));
            // if (node->command == NULL)
            // {
            //     perror("ash");
            //     exit(1);
            // }
            // strcpy(node->command, args->data[0]);
            // for (int i=1; i<(args->length-1); i++)
            // {
            //     strcat(node->command, " ");
            //     strcat(node->command, args->data[i]);
            // }

            // // add lexicographically to the list
            // if (PROCS_HEAD == NULL)
            // {
            //     // Empty List
            //     PROCS_HEAD = node;
            //     PROCS_TAIL = node;
            // }
            // else
            // {
            //     process *p = PROCS_HEAD;
            //     while (p != NULL)
            //     {
            //         if (strcmp(p->command, node->command) <= 0)
            //             p = p->next;
            //         else
            //             break;
            //     }

            //     if (p == NULL)
            //     {
            //         // Append to end of the list
            //         node->next = NULL;
            //         node->prev = PROCS_TAIL;

            //         PROCS_TAIL->next = node;
            //         PROCS_TAIL = node;
            //     }
            //     else if (p == PROCS_HEAD)
            //     {
            //         node->prev = NULL;
            //         node->next = PROCS_HEAD;
            //         PROCS_HEAD->prev = node;
            //         PROCS_HEAD = node;
            //     }
            //     else
            //     {
            //         // Add before p
            //         node->prev = p->prev;
            //         node->next = p;
            //         p->prev = node;
            //         if (node->prev != NULL)
            //         {
            //             node->prev->next = node;
            //         }
            //     }
            // }

            // fprintf(stderr, "[BG PROCESS] %d - %s\n", child_pid, args->data[0]);
