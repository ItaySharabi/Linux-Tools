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

void get_name(const char *file_path, char buf[])
{
    size_t path_len = strlen(file_path);

    // Extract filename from the full path:
    if (path_len > 1) { 
        int file_name_len = 0;
        // Read file path from the end until reaching '/'
        for (size_t i = path_len - 1; file_path[i] != '/'; --i) {
            ++file_name_len;
        }
        // Copy filename
        int offset_to_name = path_len - file_name_len;
        strncpy(buf, file_path + offset_to_name, path_len - 1);
    }
    else {
        strcpy(buf, file_path);
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
int count_dirs = 0;
int count_files = 0;

// is input equal to current working directory (cwd)
int input_was_pwd = 0;


/**
 * print_entry(4) Main method prints out file records just like the tool `tree` with the option -pugs
 * -pugs is a pattern: p-pattern , u - user ,  g - group ,  s - size of file. 
 * 
 * Traverse and print out all file records in the file-system (starting from the input file-path or cwd)
 * printing the information in the same pattern as in the `tree` tool.
 * 
 * @param file_path: The path of the current explored file.
 * @param status: stat structure pointer - containing infromation about the file which comes from invoking `stat` 
 *                with `file_path` as input
 * @param type: Integer representing the file's type
 * @param entry_ptr: FTW(File Tree Walk) Structure - Contains file's heirarchy information relatively to the current ftw.
 * */
int print_entry(const char *file_path, const struct stat *status, int type, struct FTW *entry_ptr) {

    if (is_hidden_path(file_path)) {
        return 0;
    }

    // `boolean` values to indicate file type (Reg file, Directory or Sub-Directory)
    int dir = 0;
    int file = 0;

    if (type == FTW_NS) {
        // Invoking stat(2) on file failed!
        return 0;
    }

    if (type == FTW_D && strcmp(file_path, ".") != 0) {
        dir = 1;
        file = 0;
        count_dirs++;
    }

    if (type == FTW_F) {
        count_files++;
        dir = 0;
        file = 1;
    }
    
    if (dir || file) {

        // Extract file information:
        // UID
        struct passwd *pws = getpwuid(status->st_uid);
        char *user_name = pws->pw_name;

        // GID
        struct group *grp = getgrgid(status->st_gid);
        char *group_name = grp->gr_name;

        // File permissions:
        mode_t mode = status->st_mode;
        char permissions[10];

        permissions[0] = type == FTW_F ? '-' : 'd';
        permissions[1] = mode & S_IRUSR ? 'r' : '-';
        permissions[2] = mode & S_IWUSR ? 'w' : '-';
        permissions[3] = mode & S_IXUSR ? 'x' : '-';
        permissions[4] = mode & S_IRGRP ? 'r' : '-';
        permissions[5] = mode & S_IWGRP ? 'w' : '-';
        permissions[6] = mode & S_IXGRP ? 'x' : '-';
        permissions[7] = mode & S_IROTH ? 'r' : '-';
        permissions[8] = mode & S_IWOTH ? 'w' : '-';
        permissions[9] = mode & S_IXOTH ? 'x' : '-';

        size_t path_size = strlen(file_path);
        char filename_buf[path_size];
        get_name(file_path, filename_buf);

        // Get file depth relative to starting directory
        int depth = entry_ptr->level;

        if (depth == 0) {
            printf("%s\n", file_path);
            return 0;
        }
        int i = 0;
        while (++i < depth) {
            printf("|  ");
        }

        off_t size = status->st_size;   // File size
        printf("└──[%s %s %s\t\t%ld]  %s\n", permissions, user_name, group_name, size, filename_buf);
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
        char seperator[2];
        seperator[0] = '/';
        pwd = getcwd(buf, SIZE); // pwd
        int size = strlen(pwd);
        char pwd2[size+1];       // pwd/
        memset(pwd2, '\0', size);
        strncat(pwd2, pwd, size);
        strncat(pwd2, seperator, strlen(seperator));

        // printf("pwd: %s\npwd2: %s\nargv[1]: %s\n", pwd, pwd2, argv[1]);

        if (strcmp(pwd, argv[1]) == 0 || strcmp(pwd2, argv[1]) == 0) {
            // Check if given path is current working directory:
            // If so - use `.` instead of the full directory path.
            input_was_pwd = 1;
            if (nftw(".", print_entry, 1, flags) == -1) {
                printf("%s [error opening dir]\n", argv[1]);
                printf("\n%d directories, %d files\n", 0, 0);
            
                exit(-1);
            }
        }
        
        else {
            if (nftw(argv[1], print_entry, 1, flags) == -1) {
                printf("%s [error opening dir]\n", argv[1]);
                printf("\n%d directories, %d files\n", 0, 0);
                exit(-1);
            }
        }
    }

    
    if (!input_was_pwd) {

        printf("input was NOT pwd!\n");
    }
    // Final tree -pugs output. Add +1 to directories count only if given input was NOT current working directory! 
    printf("\n%d directories, %d files\n", count_dirs, count_files);
    return 0;
}