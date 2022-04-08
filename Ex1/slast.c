#include <stdio.h>
#include <unistd.h>
#include <utmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>


void show_entry(struct utmp *utbuf_p) {
	
	printf("%-8.8s ", utbuf_p->ut_name); // Username
	if (strcmp(utbuf_p->ut_line, "~") == 0) {
		printf("system boot  ");
		printf("%-8.16s ", utbuf_p->ut_host); // Host name
	}
	else {
		printf("%-8.8s     ", utbuf_p->ut_line); // Command line name
		printf("%-8.16s         ", utbuf_p->ut_host); // Host name
	}
	
	long c = (utbuf_p->ut_time);
	printf("%.16s \n", ctime(&c));
}


int main(int argc, char* argv[]) {

	if (argc > 2 || argc == 1) {
	
		printf("Usage: ./slast integer\n");
		exit(-1); // Bad input error code
	}
	
	int X = atoi(argv[1]);
	if (X < 0) {
		printf("Bad input!\nUsage: ./slast integer\nYou have entered X=%d\n", X);
		exit(-1); // Bad input
	}
	
	int fd = open("/var/log/wtmp", O_RDONLY);
	
	if (fd < 0) {
		
		perror("File descriptor");
		exit(-2); // Bad file input
	}
	
	if (X == 0) {
		// last -0 prints out all the records from the wtmp file
		X = INT_MAX; 
	}
	
	struct utmp current_record;
	
	size_t size = sizeof(current_record);
	off_t offset = lseek(fd, -size, SEEK_END);
	
	int finished_reading = 0;
	
	while (X > 0 && offset >= 0) { 
	
		read(fd, &current_record, size);
		if (current_record.ut_type != RUN_LVL) { // Root-lvl records are discarded.
			show_entry(&current_record);
			X--;
		}
		offset = lseek(fd, -2*size, SEEK_CUR); // Move 2 time the size of the utmp struct
		
		if (offset <= size) { // if we hit the last utmp record:
			finished_reading = 1;
		}
	}
        
        if (!finished_reading) {
        	lseek(fd, 0, SEEK_SET); // Go to beginning of the file.
        	read(fd, &current_record, size);
        }
	time_t time_created = current_record.ut_time;
	printf("\nwtmp begins ");
	printf("%.24s\n", ctime(&time_created));
	return 0;
}
