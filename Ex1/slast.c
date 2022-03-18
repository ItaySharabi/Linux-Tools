#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define SIZE 1024
#define WTMP_FILE_PATH "/mnt/d/VisualStudio Projects/cpp/Advanced-Programming/Ex1/last_out.txt"
#define OUTPUT_FILE_PATH "/mnt/d/VisualStudio Projects/cpp/Advanced-Programming/Ex1/output.txt"
#define ASCII_MIN 31
#define ASCII_MAX 127

void execute_task(const int X) {
    int fd_in = open(WTMP_FILE_PATH, O_RDONLY);
    // 0666 (Octal base) read-write permissions
    int fd_out = open(OUTPUT_FILE_PATH, O_CREAT|O_RDWR|O_TRUNC, 0666); // rw permissions
    printf("fd_in: %d | fd_out %d\n", fd_in, fd_out);

    if (fd_in < 0 || fd_out < 0) {
        perror("File descriptor");
    }

    char buf[1];
    int bytes_read, bytes_written;
    
    // Copy contents of 
    while ((bytes_read = read(fd_in, &buf, 1)) > 0) {
        // printf("%d bytes were read\n", bytes_read);

        if (buf[0] > ASCII_MIN && buf[0] < ASCII_MAX) {
            if (buf[0] == 10) {
                printf("\\n found\n");
                bytes_written = write(fd_out, "\n", 1);
            }
            printf("Char: %d\n", buf[0]);
            bytes_written = write(fd_out, &buf, 1);
        }
        // printf("%d bytes were written\n", bytes_written);
    }
}







int main(int argc, char* argv[]) {
    printf("~~~~~lseek() tool~~~~~\n");
    // This tool prints the last X lines from the ORIGINAL output
    // of `last` command.
    // The `last` command's output, is data from /var/log/wtmp file


    // Input check:
    if (argc > 2 || argc == 1) {
        printf("Usage: ./tool Integer\n");
        exit(-1); // Input error
    }

    printf("arg[1] = %s\n", argv[1]);
    
    int X = atoi(argv[1]);
    if (X < 0) {
        printf("Usage: ./tool Integer");
        exit(-1);
    }

    // Start the tool:
    execute_task(X); 
    return 0;
}