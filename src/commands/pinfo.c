#include "../headers.h"
#include "pinfo.h"

#define PROC_GRP_ID_INDEX 5
#define TERM_GRP_ID_INDEX 8

int inbuilt_pinfo(command *cmd)
{
    pid_t proc_id;

    if (cmd->length == 1)
    {
        proc_id = getpid();
    }
    else
    {
        char *end = NULL;
        proc_id = strtol(cmd->data[1], &end, 10);
        if (*end || proc_id < 0)
        {
            fprintf(stderr, "pinfo: the argument should be a valid pid\n");
            return -1;
        }
    }

    // process grp id : 5
    // terminal grp id : 8

    char proc_dir_path[PATH_MAX];
    char proc_stat_file_path[PATH_MAX];
    char proc_status_file_path[PATH_MAX];
    char proc_exe_file_path[PATH_MAX];

    memset(proc_dir_path, '\0', PATH_MAX);
    memset(proc_stat_file_path, '\0', PATH_MAX);
    memset(proc_status_file_path, '\0', PATH_MAX);
    memset(proc_exe_file_path, '\0', PATH_MAX);

    sprintf(proc_dir_path, "/proc/%d", proc_id);

    sprintf(proc_stat_file_path, "%s/stat", proc_dir_path);
    sprintf(proc_status_file_path, "%s/status", proc_dir_path);
    sprintf(proc_exe_file_path, "%s/exe", proc_dir_path);

    FILE *proc_file = fopen(proc_status_file_path, "r");
    if (proc_file == NULL)
    {
        fprintf(stderr, "%d: could not access this process id\n", proc_id);
        return -1;
    }

    char *vm_size_value = (char*)calloc(1000, sizeof(char*));
    if (vm_size_value == NULL)
    {
        perror("pinfo");
        return -1;
    }
    char proc_state = '\0';

    char *file_line = NULL;
    size_t line_size = 0, read = 0;
    while ((read = getline(&file_line, &line_size, proc_file)) != -1)
    {
        if (strncmp("VmSize:", file_line, 7) == 0)
        {
            strcpy(vm_size_value, file_line+8);
        }
        else if (strncmp("State:", file_line, 6) == 0)
        {
            proc_state = file_line[7];
        }
    }
    fclose(proc_file);

    char *proc_executable_path = calloc(PATH_MAX, sizeof(char));
    if (proc_executable_path == NULL)
    {
        perror("pinfo");
        if (vm_size_value != NULL)          free(vm_size_value);
    }

    if (readlink(proc_exe_file_path, proc_executable_path, PATH_MAX) < 0)
    {
        perror("pinfo");
        if (proc_executable_path != NULL)   free(proc_executable_path);
        if (vm_size_value != NULL)          free(vm_size_value);
        return -1;
    }

    int vm_size_offset = 0;
    while ( vm_size_value[vm_size_offset] == ' ' \
            || vm_size_value[vm_size_offset] == '\t' )
        vm_size_offset += 1;

    vm_size_value[strlen(vm_size_value)-1] = '\0';

    int is_foreground_process = 0;
    FILE *stat_file = fopen(proc_stat_file_path, "r");
    if (stat_file == NULL)
    {
        fprintf(stderr, "%d: could not access the stat file for this process id\n", proc_id);
        return -1;
    }
    char proc_grp_id[1000] = {'\0'};
    char term_grp_id[1000] = {'\0'};

    for (int stat_i=0; ; stat_i++)
    {
        fscanf(stat_file, "%s", file_line);
        if (stat_i+1 == PROC_GRP_ID_INDEX)
        {
            strcpy(proc_grp_id, file_line);
        }
        else if (stat_i+1 == TERM_GRP_ID_INDEX)
        {
            strcpy(term_grp_id, file_line);
            break;
        }
    }
    fclose(stat_file);

    is_foreground_process = (strcmp(proc_grp_id, term_grp_id) == 0 ? 1 : 0);

    printf("pid -- %d\n", proc_id);
    printf("Process Status -- %c", proc_state);
    if (is_foreground_process == 1)
    {
        printf("+");
    }
    printf("\n");
    printf("memory -- %s {Virtual Memory}\n", vm_size_value+vm_size_offset);
    printf("Executable Path -- %s\n", proc_executable_path);

    if (proc_executable_path != NULL)   free(proc_executable_path);
    if (vm_size_value != NULL)          free(vm_size_value);

    return 0;
}
