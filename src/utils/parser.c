#include "../headers.h"
#include "parser.h"

command_list* new_command_list()
{
    command_list *p = calloc(1, sizeof(command_list));

    if (p == NULL)
    {
        perror("Parsing Input");
        exit(1);
    }

    p->data = NULL;
    p->length = 0;

    return p;
}

command_list* parse_input(char *user_input)
{
    command_list *cmd_list = new_command_list();

    int cmd_count = 0;
    int cmd_end = 1;

    for (int i=0; user_input[i]!='\0'; i++)
    {
        if (user_input[i] == ';')
        {
            cmd_end = 1;
            continue;
        }

        else if (cmd_end == 1 && user_input[i] != ' ' && user_input[i] != '\t')
        {
            cmd_end = 0;
            cmd_count += 1;
        }

    }

    cmd_list->length = cmd_count;
    if (cmd_count == 0)
    {
        return cmd_list;
    }

    cmd_list->data = calloc(cmd_count, sizeof(command*));
    if (cmd_list->data == NULL)
    {
        perror("Parse Input");
        exit(1);
    }

    char *ui_copy = user_input;
    char *cmd = NULL;
    int cmd_num = 0;

    while ((cmd = strtok_r(ui_copy, ";", &ui_copy)) != NULL)
    {
        command *p = parse_command(cmd);

        if (p != NULL)
        {
            cmd_list->data[cmd_num] = p;
            cmd_num += 1;
        }
    }

    return cmd_list;
}

command* new_command()
{
    command* p = calloc(1, sizeof(command));
    if (p == NULL)
    {
        perror("Parse Input");
        exit(1);
    }

    p->data = NULL;
    p->length = 0;

    return p;
}

command* parse_command(char* input)
{
    int arg_count = 0;
    int arg_end = 1;

    for (int i=0; input[i]!='\0'; i++)
    {
        if (input[i] == ' ' || input[i] == '\t')
        {
            arg_end = 1;
            continue;
        }

        else if (arg_end == 1)
        {
            arg_end = 0;
            arg_count += 1;
        }
    }

    if (arg_count == 0)
    {
        return NULL;
    }

    command *cmd = new_command();

    cmd->length = arg_count;

    cmd->data = calloc(arg_count, sizeof(char*));
    if (cmd->data == NULL)
    {
        perror("Parse Input");
        exit(1);
    }

    // printf("Found %d arguments\n", arg_count);

    char *i_copy = input;
    char *arg = NULL;
    int arg_num = 0;

    while ((arg = strtok_r(i_copy, " \t", &i_copy)) != NULL)
    {
        cmd->data[arg_num] = strdup(arg);
        arg_num += 1;
    }

    return cmd;
}
