#include "../headers.h"
#include "prompt.h"

void prompt(){

    // Getting HOSTNAME
    char hostname[256] = {0};
    gethostname(hostname, 256);

    // Getting USERNAME
    struct passwd *pw;
    uid_t uid;

    uid = geteuid();
    pw = getpwuid(uid);

    if(pw == NULL && errno != 0)
    {
        fprintf(stderr, "Failed to get the user.\nThe shell is exiting...\n");
        exit(1);
    }

    // Getting Current Directory
    extern char* HOME_DIR;

    int HOME_DIR_len = strlen(HOME_DIR);

    char* current_dir = (char*)calloc(PATH_MAX, sizeof(char));
    if(getcwd(current_dir, PATH_MAX) == NULL)
    {
        fprintf(stderr, "Failed to read the current directory. The shell will exit now...\n");
        exit(1);
    }
    int current_dir_len = strlen(current_dir);

    char* printable_dir = (char*)calloc(PATH_MAX, sizeof(char));

    char* match = strstr(current_dir, HOME_DIR);
    if (match == current_dir \
            && (current_dir[HOME_DIR_len] == '/' || current_dir_len == HOME_DIR_len) ){
        strcpy(printable_dir, "~");
        strcat(printable_dir, current_dir+HOME_DIR_len);
    }
    else {
        strcpy(printable_dir, current_dir);
    }

    // < (Yellow)
    printf("\x1b[33;1m<\x1b[0m");

    // USERNAME@HOSTNAME (Green)
    printf("\x1b[32;1m%s@%s\x1b[0m", pw->pw_name, hostname);

    // : (Yellow)
    printf("\x1b[33;1m:\x1b[0m");

    // DIRECTORY (Blue)
    printf("\x1b[34;1m%s", printable_dir);

    // > (Yellow)
    printf("\x1b[33;1m>\x1b[0m ");

    free(current_dir);
    free(printable_dir);

}
