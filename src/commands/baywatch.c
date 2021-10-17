#include "../headers.h"
#include "../utils/terminal_modes.h"
#include "baywatch.h"

int baywatch_interrupt(int);
int baywatch_newborn(int);
int baywatch_dirty(int);

int IS_VALID_COMMAND(char *s)
{

    if (strcmp(s, "interrupt") == 0)
        return 1;
    else if (strcmp(s, "newborn") == 0)
        return 1;
    else if (strcmp(s, "dirty") == 0)
        return 1;
    return 0;
}

int inbuilt_baywatch(command *cmd)
{
    if (cmd->length != 4)
    {
        fprintf(stderr, "baywatch: expected exactly 3 arguments\n");
        return -1;
    }

    int interval = -1;
    char *end = NULL;

    int opt;
    optind = 0;
    while ((opt = getopt(cmd->length, cmd->data, "n:")) != -1)
    {
        switch (opt)
        {
            case 'n':
                interval = strtoll(optarg, &end, 10);
                if (*end || interval <= 0)
                {
                    fprintf(stderr, "baywatch: the time interval should be positive integer\n");
                    return -1;
                }
                break;
            default:
                return -1;
        }
    }

    char *option = cmd->data[optind];

    if (IS_VALID_COMMAND(option) == 0)
    {
        fprintf(stderr, "baywatch: unknown command %s\n", option);
        return -1;
    }

    int fork_ret = fork();

    if (fork_ret < 0)
    {
        perror("baywatch");
        return -1;
    }
    else if (fork_ret == 0)
    {
        // Child Process
        signal(SIGINT, SIG_DFL);
        if (strcmp(option, "interrupt") == 0)
        {
            baywatch_interrupt(interval);
            exit(0);
        }

        else if (strcmp(option, "newborn") == 0)
        {
            baywatch_newborn(interval);
            exit(0);
        }

        else if (strcmp(option, "dirty") == 0)
        {
            baywatch_dirty(interval);
            exit(0);
        }
    }
    else
    {
        // Parent Process
        enable_terminal_raw_mode();
        char inp = '\0';
        int read_ret = 1;
        int keep_running = 1;
        while (keep_running == 1 && read_ret == 1)
        {
            while ((read_ret = read(STDIN_FILENO, &inp, 1)) == 1)
            {
                if (!iscntrl(inp) && inp == 'q')
                {
                    if (kill(fork_ret, SIGKILL) != 0)
                    {
                        perror("baywatch");
                        exit(1);
                    }
                    keep_running = 0;
                    break;
                }
            }
        }
        printf("\n");
        waitpid(fork_ret, NULL, WUNTRACED);
        disable_terminal_raw_mode();
    }

    return 0;
}

int baywatch_interrupt(int interval)
{
    FILE *interrupt_file = fopen("/proc/interrupts", "r");
    if (interrupt_file == NULL)
    {
        perror("interrupts file");
        fprintf(stderr, "Execution ended. Press q to exit.\n");
        return -1;
    }

    char buf[1000];
    fgets(buf, 1000, interrupt_file);

    int offset = 0;
    while (buf[offset] != 'C')  offset++;

    printf("%s", buf+offset);

    fclose(interrupt_file);

    while (1)
    {
        interrupt_file = fopen("/proc/interrupts", "r");
        if (interrupt_file == NULL)
        {
            perror("interrupts file");
            fprintf(stderr, "Execution ended. Press q to exit.\n");
            return -1;
        }

        int found_target_line = 0;

        while (found_target_line != 1)
        {
            fgets(buf, 1000, interrupt_file);

            if (strstr(buf, "i8042") != NULL && strstr(buf, "1:") != NULL)
            {
                found_target_line = 1;
            }
        }

        char *line_end = strstr(buf, "IR");
        line_end[0] = '\0';

        offset = 0;
        while (buf[offset] != ':') offset++;
        offset++;
        while (isspace(buf[offset])) offset++;

        printf("%s\n", buf+offset);

        fclose(interrupt_file);

        sleep(interval);
    }

    return 0;
}

int baywatch_newborn(int interval)
{
    char buf[1000];
    while (1)
    {
        FILE *loadavg_file = fopen("/proc/loadavg", "r");
        if (loadavg_file == NULL)
        {
            perror("loadavg file");
            fprintf(stderr, "Execution ended. Press q to exit.\n");
            return -1;
        }

        for (int scan_num=0; scan_num<5; scan_num++)
        {
            fscanf(loadavg_file, "%s", buf);
        }
        printf("%s\n", buf);

        fclose(loadavg_file);

        sleep(interval);
    }
    return 0;
}

int baywatch_dirty(int interval)
{
    char buf[1000];
    while (1)
    {
        FILE *meminfo_file = fopen("/proc/meminfo", "r");
        if (meminfo_file == NULL)
        {
            perror("meminfo file");
            fprintf(stderr, "Execution ended. Press q to exit.\n");
            return -1;
        }

        int dirty_line_found = 0;
        while (dirty_line_found != 1)
        {
            fgets(buf, 1000, meminfo_file);
            if (strstr(buf, "Dirty:") != NULL)
            {
                dirty_line_found = 1;
            }
        }

        int offset = 6;
        while (isspace(buf[offset])) offset++;

        printf("%s", buf+offset);

        fclose(meminfo_file);

        sleep(interval);
    }
    return 0;
}
