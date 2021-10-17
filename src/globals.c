/*

   This file contains global variables to be used across the shell code

*/

#include "headers.h"

// Directories
char *HOME_DIR;
char *PREV_DIR;
char *HISTORY_FILE;

// Status
int EXIT_SHELL = 0;

process *PROCS_HEAD, *PROCS_TAIL;
long long int NUM_JOBS = 0;
