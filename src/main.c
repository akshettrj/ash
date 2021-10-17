#include "headers.h"

#include "utils/parser.h"
#include "utils/prompt.h"
#include "utils/run_command.h"
#include "utils/history.h"
#include "utils/terminal_modes.h"

void sigchildhandler(int);

void print_cmd_list(command_list *cl)
{
    printf("Found %d commands\n", cl->length);

    for (int i=0; i<cl->length; i++)
    {
        printf("%d.", i+1);
        command *c = cl->data[i];
        for (int j=0; j<c->length; j++)
        {
            printf(" |%s|", c->data[j]);
        }
        printf("\n");
    }
    printf("==========================\n");
}

int main(int argc, char** argv){

    // From globals.c
    extern int EXIT_SHELL;
    extern char *HOME_DIR;
    extern char *PREV_DIR;
    extern process *PROCS_HEAD;
    extern process *PROCS_TAIL;

    PREV_DIR = (char*)calloc(PATH_MAX, sizeof(char));
    HOME_DIR = (char*)calloc(PATH_MAX, sizeof(char));

    getcwd(HOME_DIR, PATH_MAX);

    size_t command_length = ASH_MAX_COMMAND_LENGTH;
    char *input_buffer = (char*)calloc(command_length, sizeof(char));
    char escape_code[3];
    char c;

    initialize_history_file();

    PROCS_HEAD = NULL;
    PROCS_TAIL = NULL;

    signal(SIGCHLD, sigchildhandler);
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    // Run the shell indefinitely
    while(1)
    {
        setbuf(stdout, NULL);
        prompt();
        enable_terminal_raw_mode();
        memset(input_buffer, '\0', command_length);
        int pt = 0;

        int keep_reading = 1;
        while (keep_reading == 1 && read(STDIN_FILENO, &c, 1) == 1)
        {
            if (!iscntrl(c))
            {
                input_buffer[pt++] = c;
                printf("%c", c);
                continue;
            }
            if (c == 127)
            {
                // DEL
                if (pt <= 0) continue;

                if (input_buffer[pt-1] == 9)
                {
                    for (int i=0; i<7; i++)
                        printf("\b");
                }
                input_buffer[--pt] = '\0';
                printf("\b \b");

                continue;
            }
            switch(c){
                case 4:
                    // End of Transmission
                    printf("\nThanks for using ash\n\t- JINDAL TECH SOLUTIONS\n");
                    exit(0);
                    break;
                case 9:
                    // TAB
                    input_buffer[pt++] = c;
                    for (int i=0; i<8; i++) printf(" ");
                    break;
                case 10:
                    // New Line
                    printf("\n");
                    keep_reading = 0;
                    break;
                case 12:
                    if (pt != 0)
                        break;
                    // run_command_old("clear");
                    keep_reading = 0;
                    break;
                case 27:
                    // Escape
                    escape_code[2] = 0;
                    if (read(STDIN_FILENO, escape_code, 2) == 2)
                    {
                        // Arrows
                    }
                    break;
                default:
                    printf("%c\n", c);

            }
        }
        disable_terminal_raw_mode();
        input_buffer[pt] = '\0';

        save_to_history_file(input_buffer);

        if (keep_reading == 1)
        {
            exit(1);
        }

        char *input_copy = strdup(input_buffer);
        command_list *cl = parse_input(input_copy);
        free(input_copy);
        // print_cmd_list(cl);

        run_commands(cl);

        for (int i=0; i<cl->length; i++)
        {
            command *cmd = cl->data[i];
            for (int j=0; j<cmd->length; j++)
            {
                free(cmd->data[j]);
            }
            free(cmd->data);
            free(cmd);
        }
        free(cl->data);
        free(cl);

    }

    free(input_buffer);
    free(HOME_DIR);
    free(PREV_DIR);

}

void sigchildhandler(int signum)
{

    extern process *PROCS_HEAD;
    extern process *PROCS_TAIL;

    process *p = NULL;

    pid_t child_pid;
    int status;
    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        p = PROCS_HEAD;
        while (p != NULL)
        {
            if (p->pid != child_pid)
            {
                p = p->next;
                continue;
            }

            // Remove the child from the child proc list
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


            if (!WEXITSTATUS(status) && WIFEXITED(status))
            {
                printf("%s with pid %d exited normally\n", p->command, p->pid);
            }
            else
            {
                printf("%s with pid %d exited with code %d\n", p->command, p->pid, WEXITSTATUS(status));
            }

            // prompt();
            free(p->command);
            free(p);
            break;
        }
    }
}
