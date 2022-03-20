#include <stdio.h>
#include <utmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>


void show_entry(struct utmp *entry_p) {

    if (entry_p->ut_type == RUN_LVL) {
        return; // Discard run_level system log in
    }

    printf("%-8.8s | \t", entry_p->ut_name);
    printf("%d | \t", entry_p->ut_type);
    printf("%d | \t", entry_p->ut_pid);

    printf("%-8.8s | \t", entry_p->ut_id);

    if (strcmp(entry_p->ut_line, "~") == 0) {
        printf("system boot | \t");
    }

    else {
        printf("%-8.8s | \t", entry_p->ut_line);
    }

    printf("%-8.16s | \t", entry_p->ut_host);
    printf("%-8.20s | \t", ctime(&entry_p->ut_time));
    printf("\n--------------------------------------------\n");
}


int main(int argc, char* argv[]) {


    if (argc > 2 || argc == 1) {
        printf("Usage: ./slast integer\n");
        exit(-1); // Bad input error code
    }

    int X = atoi(argv[1]);

    if (X <= 0) {
        printf("Usage: ./slast integer\n");
        exit(-1); // Bad input error code
    }

    int fd = open("/var/log/wtmp", O_RDONLY);

    if (fd < 0) {
        perror("File descriptor"); // Prints real error description using errno.h
        exit(-2); // Bad file input
    }

    struct utmp current_record;

    printf("Struct utmp size: %d\n", sizeof(current_record));

    int size = sizeof(current_record);

    while (read(fd, &current_record, size) == size) {
        show_entry(&current_record);
    }

    printf("\n-----------------------\n Finished reading... \n----------------------------------\n");

}