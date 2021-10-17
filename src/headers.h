#ifndef __ASH_HEADERS_H
#define __ASH_HEADERS_H

#include "consts.h"

#include <stdio.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <errno.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <grp.h>
#include <time.h>
#include <termios.h>
#include <ctype.h>
#include <libgen.h>

typedef struct process
{
    struct process *prev;
    pid_t pid;
    char *command;
    long long int job_number;
    struct process *next;
} process;

typedef struct command
{
    char **data;
    int length;
} command;

typedef struct command_list
{
    command **data;
    int length;
} command_list;

#endif
