#ifndef __ASH_COMMAND_PARSER
#define __ASH_COMMAND_PARSER

#include "../headers.h"

command* new_command();
command* parse_command(char*);

command_list* new_command_list();
command_list* parse_input(char*);

#endif
