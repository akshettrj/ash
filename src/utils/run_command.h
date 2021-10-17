#ifndef __ASH_COMMAND_RUNNER_H
#define __ASH_COMMAND_RUNNER_H

#include "../headers.h"

int run_command_old(char*);

int run_command(command*);
int run_commands(command_list*);

#endif
