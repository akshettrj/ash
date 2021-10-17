#ifndef __ASH_CHILD_PROCESSES_LIST_H
#define __ASH_CHILD_PROCESSES_LIST_H

#include "../headers.h"

process* new_process_node();
void add_to_jobs_list(pid_t, command*);

#endif
