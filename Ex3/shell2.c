#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

int main() {
char command[1024];
char *token;
char *outfile;
int i, fd, amper, redirect_STD_OUT = 0, redirect_STD_ERR = 0, retid, status, size;
char *argv[10];

while (1)
{
    printf("hello: ");
    fgets(command, 1024, stdin);
    printf("Command: %s\n\n", command);
    command[strlen(command) - 1] = '\0';
    size = strlen(command);

    /* parse command line */
    i = 0;
    token = strtok (command," ");
    while (token != NULL)
    {
        // printf("Token: %s\n", token);
        argv[i] = token;
        token = strtok (NULL, " ");
        i++;
    }
    argv[i] = NULL;

    /* Is command empty */
    if (argv[0] == NULL)
        continue;

    /* Does command line end with & */ 
    if (! strcmp(argv[i - 1], "&")) {
        amper = 1;
        argv[i - 1] = NULL;
    }
    else 
        amper = 0; 

    /* Redirections */
    if (! strcmp(argv[i - 2], "2>")) {
        redirect_STD_ERR = 1;
        argv[i - 2] = NULL;
        outfile = argv[i - 1];
        // printf("Outfile: %s\n", outfile);
    }

    else if (! strcmp(argv[i - 2], ">")) {
        redirect_STD_OUT = 1;
        argv[i - 2] = NULL;
        outfile = argv[i - 1];
    }

    /* for commands not part of the shell command language */ 

    if (fork() == 0) { 
        /* redirection of IO*/
        if (redirect_STD_OUT) {
            fd = creat(outfile, 0660); 
            close (STDOUT_FILENO) ; 
            dup(fd); 
            close(fd); 
            /* stdout is now redirected */
        }
        else if (redirect_STD_ERR) {
            fd = creat(outfile, 0660); 
            close (STDERR_FILENO) ; 
            dup(fd); 
            close(fd);
            /* stderr is now redirected */
        }
        printf("command: %s\n", command);
        for (int i = 0; i < strlen(command); ++i) {
            printf("argv[%d] = %s\n", i, argv[i]);
        }
        execvp(argv[0], argv);
    }
    /* parent continues here */
    if (amper == 0)
        retid = wait(&status);
}
}
