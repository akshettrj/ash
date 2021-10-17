#include "../headers.h"
#include "../utils/process_path.h"
#include "ls.h"

typedef struct dirents {
    struct dirent *de;
    struct dirents *next;
} dirents;

int ls_file(char*, int, int);
int ls_dir(char*, int, int);

void print_file_type(struct stat);
void print_file_perms(struct stat);
void print_file_nlinks(struct stat, int);
void print_file_owners(struct stat, int, int);
void print_file_size(struct stat, int);
void print_file_time(struct stat);

dirents* new_dirent_node(struct dirent*);

int inbuilt_ls(command *cmd)
{
    int return_status = 0;
    int l_flag = 0, a_flag = 0;

    int opt;
    optind = 0;
    while ((opt = getopt(cmd->length, cmd->data, "la")) != -1)
    {
        switch(opt)
        {
            case 'l':
                l_flag = 1;
                break;
            case 'a':
                a_flag = 1;
                break;
            default:
                return -1;
        }
    }

    if (optind == cmd->length)
    {
        // No dir argument
        char *curr_dir = calloc(PATH_MAX, sizeof(char));
        if (curr_dir == NULL)
        {
            perror("ls");
            return -1;
        }

        if (getcwd(curr_dir, PATH_MAX) == NULL)
        {
            perror("ls");
            if (curr_dir != NULL)    free(curr_dir);
            return -1;
        }
        return_status = ls_dir(curr_dir, l_flag, a_flag);
        free(curr_dir);
        return return_status;
    }

    int path_args_count = cmd->length - optind;

    for (; optind < cmd->length; optind++)
    {
        char *arg = cmd->data[optind];
        char *processed_path = process_path(arg);

        struct stat stats;
        if (lstat(processed_path, &stats) != 0)
        {
            perror(arg);
            printf("\n");
            continue;
        }

        if (S_ISDIR(stats.st_mode))
        {
            if (path_args_count > 1)    printf("%s:\n", arg);
            return_status = ls_dir(processed_path, l_flag, a_flag) && return_status;
        }
        else
        {
            return_status = ls_file(processed_path, l_flag, a_flag) && return_status;
        }

        if (optind != cmd->length-1)    printf("\n");

        free(processed_path);
    }

    return return_status;
}

int ls_dir(char *path, int l_option, int a_option)
{
    DIR *dir = NULL;
    if ((dir = opendir(path)) == NULL)
    {
        perror(path);
        return -1;
    }

    struct dirent *de;

    dirents *head_dirent = NULL, *tail_dirent = NULL;

    while ((de = readdir(dir)) != NULL)
    {
        if (a_option == 0 && de->d_name[0] == '.')  continue;

        if (l_option == 0)  printf("%s\n", de->d_name);

        else
        {
            dirents *new_de = new_dirent_node(de);

            if (head_dirent == tail_dirent && head_dirent == NULL)
            {
                head_dirent = new_de;
                tail_dirent = new_de;
            }
            else
            {
                tail_dirent->next = new_de;
                tail_dirent = new_de;
            }

        }
    }

    if (l_option == 1)
    {
        struct stat stats;

        blkcnt_t total_blocks = 0;

        int max_owner_name_len = 0;
        int max_group_name_len = 0;
        nlink_t max_nlink = 0;
        int max_nlink_len = 0;
        off_t max_size = 0;
        int max_size_len = 0;

        dirents *p = head_dirent, *temp;
        while (p != NULL)
        {
            char *file_path = (char*)calloc(PATH_MAX, sizeof(char));
            strcpy(file_path, path);
            strcat(file_path, "/");
            strcat(file_path, p->de->d_name);

            if (lstat(file_path, &stats) == -1){
                perror(file_path);
                free(file_path);
                closedir(dir);
                return -1;
            }

            max_nlink = stats.st_nlink > max_nlink ? stats.st_nlink : max_nlink;
            max_size = stats.st_size > max_size ? stats.st_size : max_size;

            struct passwd *usr = getpwuid(stats.st_uid);
            struct group *grp = getgrgid(stats.st_uid);
            if (usr == NULL || grp == NULL)
            {
                perror(file_path);
                closedir(dir);
                return -1;
            }

            int owner_name_len = strlen(usr->pw_name);
            int group_name_len = strlen(grp->gr_name);

            max_owner_name_len = owner_name_len > max_owner_name_len ? owner_name_len : max_owner_name_len;
            max_group_name_len = group_name_len > max_group_name_len ? group_name_len : max_group_name_len;

            total_blocks += stats.st_blocks;

            p = p->next;
            free(file_path);
        }

        char buf[30];

        memset(buf, '\0', 30);
        sprintf(buf, "%lld", (unsigned long long int)max_nlink);
        max_nlink_len = strlen(buf);

        memset(buf, '\0', 30);
        sprintf(buf, "%lld", (unsigned long long int)max_size);
        max_size_len = strlen(buf);

        printf("total %lld\n", (unsigned long long int)total_blocks/2);

        p = head_dirent;
        while (p != NULL)
        {
            char *file_path = (char*)calloc(PATH_MAX, sizeof(char));
            strcpy(file_path, path);
            strcat(file_path, "/");
            strcat(file_path, p->de->d_name);

            if (lstat(file_path, &stats) == -1){
                perror(file_path);
                free(file_path);
                closedir(dir);
                return -1;
            }

            print_file_type(stats);
            print_file_perms(stats);
            printf(" ");
            print_file_nlinks(stats, max_nlink_len);
            printf(" ");
            print_file_owners(stats, max_owner_name_len, max_group_name_len);
            printf(" ");
            print_file_size(stats, max_size_len);
            printf(" ");
            print_file_time(stats);
            printf(" %s", basename(file_path));

            if (S_ISLNK(stats.st_mode))
            {
                char *real_path = (char*)calloc(PATH_MAX, sizeof(char));
                realpath(file_path, real_path);
                printf(" -> %s", real_path);
                free(real_path);
            }

            printf("\n");

            temp = p;
            p = p->next;
            free(temp);
            free(file_path);
        }
    }

    closedir(dir);
    return 0;
}

int ls_file(char *path, int l_option, int a_option)
{

    if (l_option == 0)
        printf("%s\n", path);
    else
    {
        struct stat stats;
        if (lstat(path, &stats) != 0)
        {
            perror(path);
            return -1;
        }

        print_file_type(stats);
        print_file_perms(stats);
        printf(" ");
        print_file_nlinks(stats, 0);
        printf(" ");
        print_file_owners(stats, 0, 0);
        printf(" ");
        print_file_size(stats, 0);
        printf(" ");
        print_file_time(stats);
        printf(" %s", path);

        if (S_ISLNK(stats.st_mode))
        {
            char *real_path = (char*)calloc(PATH_MAX, sizeof(char));
            realpath(path, real_path);
            printf(" -> %s", real_path);
            free(real_path);
        }

        printf("\n");
    }

    return 0;
}

void print_file_type(struct stat st)
{
    char file_type = '-';

    if (S_ISBLK(st.st_mode))
    {
        file_type = 'b';        // Block Special file
    }
    else if (S_ISCHR(st.st_mode))
    {
        file_type = 'c';        // Character Special file
    }
    else if (S_ISDIR(st.st_mode))
    {
        file_type = 'd';        // Directory
    }
    else if (S_ISFIFO(st.st_mode))
    {
        file_type = 'p';        // FIFO
    }
    else if (S_ISLNK(st.st_mode))
    {
        file_type = 'l';        // Symbolic Link
    }
    else if (S_ISSOCK(st.st_mode))
    {
        file_type = 'n';        // Network File
    }
    else if (S_ISREG(st.st_mode))
    {
        file_type = '-';        // Regular File
    }

    printf("%c", file_type);
}

void print_file_perms(struct stat st)
{
    (st.st_mode & S_IRUSR) ? printf("r") : printf("-");
    (st.st_mode & S_IWUSR) ? printf("w") : printf("-");
    (st.st_mode & S_IXUSR) ? printf("x") : printf("-");
    (st.st_mode & S_IRGRP) ? printf("r") : printf("-");
    (st.st_mode & S_IWGRP) ? printf("w") : printf("-");
    (st.st_mode & S_IXGRP) ? printf("x") : printf("-");
    (st.st_mode & S_IROTH) ? printf("r") : printf("-");
    (st.st_mode & S_IWOTH) ? printf("w") : printf("-");
    (st.st_mode & S_IXOTH) ? printf("x") : printf("-");
}

void print_file_nlinks(struct stat st, int max_nlink_len)
{
    char buf[30] = {0};
    memset(buf, '\0', 20);
    sprintf(buf, "%lld", (unsigned long long int)st.st_nlink);

    int cur_nlink_len = strlen(buf);

    int spaces_req = max_nlink_len > 0 ? max_nlink_len - cur_nlink_len : 0;
    for (int i=0; i<spaces_req; i++) printf(" ");

    printf("%lld", (unsigned long long int)st.st_nlink);
}

void print_file_owners(struct stat st, int max_owner_len, int max_group_len)
{
    struct passwd *usr = getpwuid(st.st_uid);
    struct group *grp = getgrgid(st.st_uid);
    if (usr == NULL || grp == NULL)
    {
        perror("");
    }

    int owner_len = strlen(usr->pw_name);
    int group_len = strlen(grp->gr_name);

    int owner_spaces_req = max_owner_len > 0 ? max_owner_len - owner_len : 0;
    int group_spaces_req = max_group_len > 0 ? max_group_len - group_len : 0;

    for (int i=0; i<owner_spaces_req; i++) printf(" ");
    printf("%s", usr->pw_name);

    printf(" ");

    for (int i=0; i<group_spaces_req; i++) printf(" ");
    printf("%s", grp->gr_name);
}

void print_file_size(struct stat st, int max_size_len)
{
    char buf[30] = {0};
    memset(buf, '\0', 20);
    sprintf(buf, "%lld", (unsigned long long int)st.st_size);

    int cur_size_len = strlen(buf);

    int spaces_req = max_size_len > 0 ? max_size_len - cur_size_len : 0;
    for (int i=0; i<spaces_req; i++) printf(" ");

    printf("%lld", (unsigned long long int)st.st_size);
}

void print_file_time(struct stat st)
{
    int show_only_year = 0;

    time_t curr_time_t;
    time(&curr_time_t);
    struct tm curr_tm;
    localtime_r(&curr_time_t, &curr_tm);

    struct tm mtime_tm;
    localtime_r(&st.st_mtim.tv_sec, &mtime_tm);
    time_t mtime_time_t = mktime(&mtime_tm);

    double time_diff = difftime(curr_time_t, mtime_time_t);

    if (time_diff < 0)
    {
        show_only_year = 1;
    }
    else if (curr_tm.tm_year - mtime_tm.tm_year > 1)
    {
        show_only_year = 1;
    }
    else
    {
        int curr_month = curr_tm.tm_mon;
        int mtime_month = mtime_tm.tm_year == curr_tm.tm_year \
                          ? mtime_tm.tm_mon \
                          : mtime_tm.tm_mon - 12;

        if (curr_month - mtime_month == 6)
        {
            int curr_day = curr_tm.tm_mday;
            int mtime_day = mtime_tm.tm_mday;
            if (mtime_day <= curr_day)
            {
                show_only_year = 1;
            }
        }
        else if (curr_month - mtime_month > 6)
        {
            show_only_year = 1;
        }
    }

    char timebuf[13] = {0};
    if (show_only_year == 1)
    {
        strftime(timebuf, sizeof(timebuf), "%b %-d  %Y", &mtime_tm);
    }
    else
    {
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", &mtime_tm);
    }

    if (timebuf[4] == '0')
        timebuf[4] = ' ';

    printf("%s", timebuf);
}

dirents* new_dirent_node(struct dirent *de)
{
    dirents *node = (dirents*)calloc(1, sizeof(dirents));
    if (node == NULL)
    {
        perror("Calloc failed");
        exit(-1);
    }

    node->next = NULL;
    node->de = de;

    return node;
}
