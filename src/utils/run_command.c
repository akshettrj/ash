#include "../headers.h"
#include "run_command.h"
#include "create_process.h"
#include "parser.h"
#include "process_path.h"

#include "../commands/baywatch.h"
#include "../commands/bg.h"
#include "../commands/cd.h"
#include "../commands/fg.h"
#include "../commands/ls.h"
#include "../commands/pwd.h"
#include "../commands/jobs.h"
#include "../commands/echo.h"
#include "../commands/pinfo.h"
#include "../commands/replay.h"
#include "../commands/sig.h"
#include "../commands/history.h"

static inline int IS_REDIRECTION_OR_PIPE_SYMBOL(char *x)
{
    if (strcmp(x, ">") == 0)
        return 1;
    if (strcmp(x, ">>") == 0)
        return 1;
    if (strcmp(x, "<") == 0)
        return 1;
    if (strcmp(x, "|") == 0)
        return 1;

    return 0;
}

int run_commands(command_list* cmd_list)
{
    int return_status = 0;

    for (int i=0; i<cmd_list->length; i++)
    {
        command *curr_cmd = cmd_list->data[i];

        // Check for presence of piping
        int has_pipes = 0;
        int last_was_pipe = 0;
        for (int j=0; j<curr_cmd->length; j++)
        {
            if (strcmp(curr_cmd->data[j], "|") == 0)
            {
                if (last_was_pipe == 1 || j == 0 || j+1 == curr_cmd->length)
                {
                    fprintf(stderr, "ash: parsing error, could not parse the command near |\n");
                    return -1;
                }
                last_was_pipe = 1;
                has_pipes = 1;
            }
            else
            {
                last_was_pipe = 0;
            }
        }

        // Run normally if no pipes
        if (has_pipes == 0)
        {
            return_status = run_command(curr_cmd) && return_status;
            continue;
        }

        // Count the number of pipes
        int number_of_pipes = 0;
        for (int j=0; j<curr_cmd->length; j++)
        {
            if (strcmp(curr_cmd->data[j], "|") == 0)
            {
                number_of_pipes += 1;
            }
        }

        // Iterate over pipes
        int pipe_fd[2] = {-1,-1};
        int input_fd = STDIN_FILENO;
        int original_stdout = dup(STDOUT_FILENO);
        int original_stdin = dup(STDIN_FILENO);
        int arg_num = 0;

        int fork_pids[number_of_pipes+1];

        for (int pipe_i=0; pipe_i<number_of_pipes+1; pipe_i++)
        {
            command *pipe_sub_cmd = new_command();
            int starting_index = arg_num, arg_i = arg_num;
            while (arg_i < curr_cmd->length && strcmp(curr_cmd->data[arg_i], "|") != 0)
            {
                arg_i++;
            }
            int sub_cmd_len = arg_i - arg_num;
            pipe_sub_cmd->length = sub_cmd_len;
            pipe_sub_cmd->data = calloc(sub_cmd_len, sizeof(char*));
            if (pipe_sub_cmd->data == NULL)
            {
                perror("ash");
                exit(1);
            }
            while (arg_num < curr_cmd->length && strcmp(curr_cmd->data[arg_num], "|") != 0)
            {
                pipe_sub_cmd->data[arg_num - starting_index] = curr_cmd->data[arg_num];
                arg_num++;
            }
            arg_num++;

            if (pipe(pipe_fd) < 0)
            {
                perror("error while creating pipe:");
                return -1;
            }

            pid_t fork_ret = fork();
            if (fork_ret < 0)
            {
                // Error
                perror("ash");
                return -1;
            }
            else if (fork_ret == 0)
            {
                // Child Process
                if (dup2(input_fd, STDIN_FILENO) < 0)
                {
                    perror("ash");
                    return -1;
                }
                if (pipe_i != number_of_pipes)
                {
                    if (dup2(pipe_fd[1], STDOUT_FILENO) < 0)
                    {
                        perror("ash");
                        return -1;
                    }
                }
                close(pipe_fd[0]);
                run_command(pipe_sub_cmd);
                _exit(0);
            }
            else
            {
                // Parent Process
                // wait(NULL);
                close(pipe_fd[1]);
                fork_pids[pipe_i] = fork_ret;
                input_fd = pipe_fd[0];
            }
        }
        for (int pipe_i=0; pipe_i<number_of_pipes+1; pipe_i++)
        {
            waitpid(fork_pids[pipe_i], NULL, 0);
        }
        if (dup2(original_stdin, STDIN_FILENO) < 0)
        {
            perror("ash");
        }
        close(original_stdin);
        if (dup2(original_stdout, STDOUT_FILENO) < 0)
        {
            perror("ash");
        }
        close(original_stdout);
    }

    return return_status;
}

int run_command(command *input_cmd)
{
    if (input_cmd == NULL || input_cmd->length == 0)
    {
        return -1;
    }
    command *cmd = NULL;

    int return_status = 0;
    char *cmd_str = input_cmd->data[0];

    int original_stdin = dup(STDIN_FILENO);
    int original_stdout = dup(STDOUT_FILENO);

    // Detect Redirection
    char *out_file_path = NULL, *in_file_path = NULL;
    int append_to_out_file = 0;
    for (int i=0; i<input_cmd->length; i++)
    {
        char *arg = input_cmd->data[i];

        if (strcmp(arg, ">") == 0)
        {
            if (i != (input_cmd->length-1) && IS_REDIRECTION_OR_PIPE_SYMBOL(input_cmd->data[i+1]) == 0)
            {
                out_file_path = process_path(input_cmd->data[i+1]);
                append_to_out_file = 0;
            }
            else {
                fprintf(stderr, "ash: failed to parse the output redirection file path\n");
                return_status = -1;
                goto return_label;
            }
        }
        else if (strcmp(arg, ">>") == 0)
        {
            if (i != (input_cmd->length-1) && IS_REDIRECTION_OR_PIPE_SYMBOL(input_cmd->data[i+1]) == 0)
            {
                out_file_path = process_path(input_cmd->data[i+1]);
                append_to_out_file = 1;
            }
            else {
                fprintf(stderr, "ash: failed to parse the output redirection file path\n");
                return_status = -1;
                goto return_label;
            }
        }
        else if (strcmp(arg, "<") == 0)
        {
            if (i != (input_cmd->length-1) && IS_REDIRECTION_OR_PIPE_SYMBOL(input_cmd->data[i+1]) == 0)
            {
                in_file_path = process_path(input_cmd->data[i+1]);
            }
            else {
                fprintf(stderr, "ash: failed to parse the input redirection file path\n");
                return_status = -1;
                goto return_label;
            }
        }
    }


    if (out_file_path != NULL)
    {
        int out_file_desc;
        if (append_to_out_file == 1)
            out_file_desc = open(out_file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
        else
            out_file_desc = open(out_file_path, O_WRONLY | O_TRUNC | O_CREAT, 0644);
        if (out_file_desc == -1)
        {
            perror(out_file_path);
            return_status = -1;
            goto return_label;
        }
        if (dup2(out_file_desc, STDOUT_FILENO) < 0)
        {
            perror("ash");
            return_status = -1;
            goto return_label;
        }
    }

    if (in_file_path != NULL)
    {
        int in_file_desc;
        in_file_desc = open(in_file_path, O_RDONLY);
        if (in_file_desc == -1)
        {
            perror(in_file_path);
            return_status = -1;
            goto return_label;
        }
        if (dup2(in_file_desc, STDIN_FILENO) < 0)
        {
            perror("ash");
            return_status = -1;
            goto return_label;
        }
    }

    if (in_file_path != NULL && out_file_path != NULL)
    {
        cmd = new_command();
        if (cmd == NULL)
        {
            perror("ash");
            exit(1);
        }

        cmd->length = input_cmd->length - 4;
        cmd->data = calloc(cmd->length, sizeof(char*));
        if (cmd->data == NULL)
        {
            perror("ash");
            exit(1);
        }
        int copied_count = 0;
        for (int i=0; i<input_cmd->length; i++)
        {
            char *arg = input_cmd->data[i];
            if (strcmp(arg, ">>") == 0 || strcmp(arg, ">") == 0 || strcmp(arg, "<") == 0)
            {
                i += 1;
            }
            else
            {
                cmd->data[copied_count] = input_cmd->data[i];
                copied_count += 1;
            }
        }
    }
    else if (in_file_path != NULL || out_file_path != NULL)
    {
        cmd = new_command();
        cmd->length = input_cmd->length - 2;
        cmd->data = calloc(cmd->length, sizeof(char*));
        if (cmd->data == NULL)
        {
            perror("ash");
            exit(1);
        }
        int copied_count = 0;
        for (int i=0; i<input_cmd->length; i++)
        {
            char *arg = input_cmd->data[i];
            if (strcmp(arg, ">>") == 0 || strcmp(arg, ">") == 0 || strcmp(arg, "<") == 0)
            {
                i += 1;
            }
            else
            {
                cmd->data[copied_count] = input_cmd->data[i];
                copied_count += 1;
            }
        }
    }
    else
    {
        cmd = input_cmd;
    }

    if (strcmp(cmd_str, "exit") == 0)
    {
        exit(0);
    }
    else if (strcmp(cmd_str, "baywatch") == 0)
    {
        return_status = inbuilt_baywatch(cmd);
    }
    else if (strcmp(cmd_str, "bg") == 0)
    {
        return_status = inbuilt_bg(cmd);
    }
    else if (strcmp(cmd_str, "cd") == 0)
    {
        return_status = inbuilt_cd(cmd);
    }
    else if (strcmp(cmd_str, "fg") == 0)
    {
        return_status = inbuilt_fg(cmd);
    }
    else if (strcmp(cmd_str, "echo") == 0)
    {
        return_status = inbuilt_echo(cmd);
    }
    else if (strcmp(cmd_str, "history") == 0)
    {
        return_status = inbuilt_history(cmd);
    }
    else if (strcmp(cmd_str, "jobs") == 0)
    {
        return_status = inbuilt_jobs(cmd);
    }
    else if (strcmp(cmd_str, "ls") == 0)
    {
        return_status = inbuilt_ls(cmd);
    }
    else if (strcmp(cmd_str, "pinfo") == 0)
    {
        return_status = inbuilt_pinfo(cmd);
    }
    else if (strcmp(cmd_str, "pwd") == 0)
    {
        return_status = inbuilt_pwd(cmd);
    }
    else if (strcmp(cmd_str, "repeat") == 0)
    {
        if (cmd->length < 3)
        {
            fprintf(stderr, "repeat: pass number of times to repeat and a command to repeat\n");
            return_status = -1;
            goto return_label;
        }

        char *end = NULL;
        long long int count_repeation = strtoll(cmd->data[1], &end, 10);

        if (count_repeation <= 0 || *end)
        {
            fprintf(stderr, "repeat: the number of times to repeat should be positive integer\n");
            return_status = -1;
            goto return_label;
        }

        command *cmd_to_repeat = new_command();
        cmd_to_repeat->length = cmd->length - 2;
        cmd_to_repeat->data = calloc(cmd_to_repeat->length, sizeof(char*));
        if (cmd_to_repeat->data == NULL)
        {
            perror("ash");
        }

        for (int i=0; i<(cmd->length-2); i++)
        {
            cmd_to_repeat->data[i] = cmd->data[i+2];
        }

        for (int i=0; i<count_repeation; i++)
        {
            return_status = run_command(cmd_to_repeat) && return_status;
        }

        free(cmd_to_repeat->data);
        free(cmd_to_repeat);

    }
    else if (strcmp(cmd_str, "replay") == 0)
    {
        return_status = inbuilt_replay(cmd);
    }
    else if (strcmp(cmd_str, "sig") == 0)
    {
        return_status = inbuilt_sig(cmd);
    }
    else
    {
        create_process(cmd);
    }

return_label:

    if (cmd != NULL && (out_file_path != NULL || in_file_path != NULL))
    {
        if (cmd->data != NULL)      free(cmd->data);
        if (cmd != NULL)            free(cmd);
        if (out_file_path != NULL)  free(out_file_path);
        if (in_file_path != NULL)   free(in_file_path);
    }

    if (dup2(original_stdin, STDIN_FILENO)< 0)
    {
        perror("ash");
        return -1;
    }
    if (close(original_stdin) == -1)
    {
        fprintf(stderr, "Failed to close the copy of stdin\n");
    }
    if (dup2(original_stdout, STDOUT_FILENO) < 0)
    {
        perror("ash");
        return -1;
    }
    if (close(original_stdout) == -1)
    {
        fprintf(stderr, "Failed to close the copy of stdout\n");
    }

    return return_status;
}
