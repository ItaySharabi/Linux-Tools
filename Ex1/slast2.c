#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define SIZE 1024

int file_reorder(int fd) {
    printf("\n~File Reorder~\n");
    // Creates a file called `new_file`

    int fd_out = open("/mnt/d/VisualStudio Projects/cpp/Advanced-Programming/Ex1/new_file.txt", O_CREAT|O_RDWR);
    if (fd_out < 0) {
        printf("Error opening file:\n");
        perror("File descriptor");
        exit(-1);
    }
    lseek(fd, 0, SEEK_SET); // If the file pointer is not set to 0, now it will be set.
    char buff[SIZE]; // Create a buffer
    int bytes_read;
    int bytes_written;

    while ((bytes_read = read(fd, &buff, SIZE)) > 0) {
        printf("Read from buffer: %s\n", buff);
        for (int i = 0; i < bytes_read; i++) {
            if (buff[i] > 31 && buff[i] < 127) {
                printf("Printable: %c\n", buff[i]);
            }
            else {
                printf("Not printable: %c\n", buff[i]);
            }
        }
        bytes_written = write(fd_out, &buff, bytes_read);
        printf("Written %d bytes\n", bytes_written);

        if (bytes_written == -1) {
            perror("Re-writing file failed");
            exit(-1);
        }
    }
    printf("\n--------------------------------------------------\n");
    close(fd_out);
    return 0;
}

void file_print(int fd) {
    char buf[SIZE];
    int bytes_read;

    printf("Data:\n-----------------------------------------------\n");
    while ((bytes_read = read(fd, &buf, SIZE)) > 0) {
        printf("Bytes read: %d\n", bytes_read);        
        printf("Data load:\n%s\n---------------------------------------------\n", buf);
    }
    printf("-----------------------------------------------\n");
}

void execute_task(int X) {

    // Open file: /var/log/wtmp
    // int wtmp_fd = file_open("/mnt/d/VisualStudio Projects/cpp/Advanced-Programming/Ex1/wtmp_file.txt");

    // printf("File descriptor of wtmp_file: %d\n", wtmp_fd);

    // Print out the last X lines of this file. `last`

    int fd = open("/mnt/d/VisualStudio Projects/cpp/Advanced-Programming/Ex1/last_out.txt", O_RDONLY);

    // printf("fd: %d\n", fd);
    if (fd < 0) {
        printf("An error occured opening file: \n");
        perror("File descriptor");
        exit(-1);
    }

    printf("File descriptor: %d\n", fd);

    file_print(fd); // Print file content's

    // Reorder the file's content:
    int new_file = file_reorder(fd); // Outputs a new file named: new_file

    file_print(new_file); // Print file content's
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










// void onto_seeking(int fd, int X) {
    
//     char delim_buffer[SIZE];
//     char read_buffer[SIZE];

//     int count_lines = 0;

//     off_t offset = lseek(fd, -1, SEEK_END); // Set the file's pointer to (end of file) - 1.
//     int file_size = offset;
//     printf("File size (Bytes): %d\n", file_size);
    
//     printf("--------------\nFinished iterating the file\n--------------\n");
//     printf("Current offset: %ld\nLines found: %d\n", offset, count_lines);

// }

// int number_of_lines(int fd, int X) {

//     int count = 0;
//     int bytes_read = 0;
//     char buffer[SIZE];

//     while ((bytes_read = read(fd, &buffer, SIZE)) > 0) {
//         printf("Data:\n-----------------\n%s\n-----------------\n", buffer);
//         for (int i = 0; i < SIZE; i++) {
//             if (buffer[i] == '\n') {
//                 for (int j = i; j < SIZE - 2) {
//                     printf("");
//                 }
//                 count++;
//             }
//         }
//     }
//     return count;
// }

// int open_file(const char* filepath) {

//     int fd = open(filepath, O_RDONLY);
//     if (fd < 0) {
//         perror("File descriptor");
//         exit(-1);
//     }
//     return fd;
// }

// void execute_task(int X) {

//     int fd = open_file("/mnt/d/VisualStudio Projects/cpp/Advanced-Programming/Ex1/last_out.txt");
//     printf("File descriptor: %d\n", fd);


//     int n = number_of_lines(fd, X);

//     printf("Number of lines: %d\n", n);
// }

