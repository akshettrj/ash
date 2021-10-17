#include "../headers.h"
#include "../utils/parser.h"
#include "../utils/run_command.h"
#include "replay.h"

static inline int IS_NOT_FLAG(char *s)
{
    if (strcmp(s, "-command") == 0)
        return 0;
    if (strcmp(s, "-period") == 0)
        return 0;
    if (strcmp(s, "-interval") == 0)
        return 0;
    return 1;
}

int inbuilt_replay(command *cmd)
{
    long long int interval = -1, period = -1;

    int repetition_cmd_start_index = -1, repetition_cmd_end_index = -1;

    for (int arg_i=1; arg_i<cmd->length; arg_i++)
    {
        char *arg = cmd->data[arg_i];
        if (strcmp(arg, "-interval") == 0)
        {
            if (arg_i == cmd->length-1)
            {
                fprintf(stderr, "replay: please provide the interval\n");
                return -1;
            }
            {
                char *end = NULL;
                interval = strtoll(cmd->data[arg_i+1], &end, 10);
                if (*end || interval <= 0)
                {
                    fprintf(stderr, "replay: the repetition interval should be positive number\n");
                    return -1;
                }
            }
        }
        else if (strcmp(arg, "-period") == 0)
        {
            if (arg_i == cmd->length-1)
            {
                fprintf(stderr, "replay: please provide the period\n");
                return -1;
            }
            {
                char *end = NULL;
                period = strtoll(cmd->data[arg_i+1], &end, 10);
                if (*end || period < 0)
                {
                    fprintf(stderr, "replay: the repetition period should be non-zero number\n");
                    return -1;
                }
            }
        }
        else if (strcmp(arg, "-command") == 0)
        {
            if (repetition_cmd_start_index != -1)
            {
                fprintf(stderr, "replay: the flag -command can be used only once\n");
                return -1;
            }
            if (arg_i == cmd->length-1 || IS_NOT_FLAG(cmd->data[arg_i+1]) == 0)
            {
                fprintf(stderr, "replay: pass the command to repeat\n");
                return -1;
            }
            repetition_cmd_start_index = arg_i + 1;
            repetition_cmd_end_index = arg_i + 1;
            while (repetition_cmd_end_index < cmd->length && IS_NOT_FLAG(cmd->data[repetition_cmd_end_index]) == 1)
            {
                repetition_cmd_end_index += 1;
            }
        }
    }

    if (repetition_cmd_start_index == -1 || repetition_cmd_end_index == -1)
    {
        fprintf(stderr, "replay: pass the command to repeat\n");
        return -1;
    }

    if (interval == -1)
    {
        fprintf(stderr, "replay: pass the intervals between consecutive repetitions\n");
        return -1;
    }

    if (period == -1)
    {
        fprintf(stderr, "replay: pass the period for which to repeat the command\n");
        return -1;
    }

    if (interval > period)
    {
        fprintf(stderr, "replay: the interval should not be more than the period\n");
        return -1;
    }

    // if (period % interval != 0)
    // {
    //     fprintf(stderr, "replay: the period should be divisible by the intervals between the repetitions\n");
    //     return -1;
    // }

    int number_of_repetitions = period / interval;

    // Creating the command struct for repetition
    int repetition_cmd_args_count = repetition_cmd_end_index - repetition_cmd_start_index;
    command *repetition_cmd = new_command();

    repetition_cmd->length = repetition_cmd_args_count;
    repetition_cmd->data = calloc(repetition_cmd_args_count, sizeof(char*));
    if (repetition_cmd->data == NULL)
    {
        perror("replay");
        return -1;
    }

    for (int arg_i=repetition_cmd_start_index; arg_i<repetition_cmd_end_index; arg_i++)
    {
        repetition_cmd->data[arg_i-repetition_cmd_start_index] = cmd->data[arg_i];
    }

    for (int repetition_i=0; repetition_i<number_of_repetitions; repetition_i++)
    {
        sleep(interval);
        run_command(repetition_cmd);
    }

    sleep(period % interval);

    return 0;
}
