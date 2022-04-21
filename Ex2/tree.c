#include <stdio.h>
#include <string.h>
#define _XOPEN_SOURCE 600 /* Get nftw() */
#define __USE_XOPEN_EXTENDED 1

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <ftw.h>

#define SIZE 1024

void get_name(const char *name, char buf[])
{
    size_t path_len = strlen(name);
    // char file_name[path_len]; // A buffer to hold the filename only
    // Extract filename from the full path:
    if (path_len == 1)
    {
        strcpy(buf, name);
    }
    else
    {
        int file_name_len = 0;
        for (size_t i = path_len - 1; name[i] != '/'; --i)
        {
            ++file_name_len;
        }
        int name_begin_index = path_len - file_name_len;
        strncpy(buf, name + name_begin_index, path_len - 1);
    }
}


/**
 * is_hidden_path() method returns true or false 
 * whether the given path is on a hidden file/directory path.
 * 
 * @param path: A path to a file.
 * @return true (1) - if the path contains a hidden file, i.e. the sequence: "/.*"
 *         false (0) - otherwise. 
 * */
int is_hidden_path(const char* path) {

    int size = strlen(path);
    for (int i = 0; i < size; ++i) {
        
        if (*(path + i) == '/') {
            // Beginnig of a file name:
            if (i < size-1 && *(path + i + 1) == '.') {
                // Found a hidden path:
                return 1;
            }
        }
    }
    return 0;
}

// Global counters:
int count_dirs = -1;
int count_files = 0;

int print_entry(const char *name, const struct stat *status, int type, struct FTW *ftwb) {

    if (is_hidden_path(name)) {
        return 0;
    }

    // `boolean` values to indicate file type (Reg file, Directory or Sub-Directory)
    int dir = 0;
    int file = 0;

    if (type == FTW_NS) {
        // Invoking stat(2) on file failed!
        return 0;
    }

    if (type == FTW_D && strcmp(name, ".") != 0) {
        dir = 1;
        file = 0;
        count_dirs++;
    }

    else if (strcmp(name, ".") == 0) {
        // If current directory is `.` - add it to the counter to fix the count
        count_dirs++;
    }

    if (type == FTW_F) {
        count_files++;
        dir = 0;
        file = 1;
    }
    
    if (dir || file) {

        // Extract file information:
        uid_t user_id = status->st_uid; // User ID
        gid_t gid = status->st_gid;     // Group ID
        off_t size = status->st_size;   // File size

        // UID
        struct passwd *pws = getpwuid(user_id);
        char *user_name = pws->pw_name;

        // GID
        struct group *grp = getgrgid(gid);
        char *group_name = grp->gr_name;

        // File permissions:
        char permissions[10];
        permissions[0] = type == FTW_F ? '-' : 'd';
        permissions[1] = status->st_mode & S_IRUSR ? 'r' : '-';
        permissions[2] = status->st_mode & S_IWUSR ? 'w' : '-';
        permissions[3] = status->st_mode & S_IXUSR ? 'x' : '-';
        permissions[4] = status->st_mode & S_IRGRP ? 'r' : '-';
        permissions[5] = status->st_mode & S_IWGRP ? 'w' : '-';
        permissions[6] = status->st_mode & S_IXGRP ? 'x' : '-';
        permissions[7] = status->st_mode & S_IROTH ? 'r' : '-';
        permissions[8] = status->st_mode & S_IWOTH ? 'w' : '-';
        permissions[9] = status->st_mode & S_IXOTH ? 'x' : '-';

        size_t path_size = strlen(name);
        char buf[path_size];
        get_name(name, buf);

        // get level of indentation
        int level = ftwb->level;
        // printf("FTW is now at level: %d\n", level);
        if (level == 0) {
            printf("%s\n", name);
            return 0;
        }
        int i = 1;
        while (i < level) {
            // printf("|");
            printf("|  ");
            ++i;
        }
        
        printf("└──[%s %s %s\t\t%ld]  %s\n", permissions, user_name, group_name, size, buf);
    }
    return 0;
}


int main(int argc, char *argv[]) {

    int flags = 0;

    if (argc == 1) {
        // No arguments were given:
        nftw(".", print_entry, 1, flags);
    }
    
    else {
        // Get current working directory, `pwd` in 2 variations:
        // pwd and pwd/
        char *pwd;
        char buf[SIZE];
        char *postfix;
        *postfix = '/';
        pwd = getcwd(buf, SIZE); // pwd
        int size = strlen(pwd);
        char pwd2[size+1];       // pwd/
        memset(pwd2, '\0', size);
        strncat(pwd2, pwd, size);
        strncat(pwd2, postfix, strlen(postfix));

        printf("pwd: %s\npwd2: %s\nargv[1]: %s\n", pwd, pwd2, argv[1]);

        if (strcmp(pwd, argv[1]) == 0 || strcmp(pwd2, argv[1]) == 0) {
            // Check if given path is current working directory:
            // If so - use `.` instead of the full directory path.
            nftw(".", print_entry, 1, flags);
        }
        
        else {
            nftw(argv[1], print_entry, 1, flags);
        }
    }
    printf("\n%d directories, %d files\n", count_dirs, count_files);
    return 0;
}