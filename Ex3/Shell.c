#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include "signal.h"
#include <string.h>

void sig_handler(int sig) {
    printf("You've pressed Ctrl+C!\nType `quit` to quit the shell\n");
}

void copy_into(char* buf1, char* buf2) {
    memset(buf2, '\0', 1024);
    strncpy(buf2, buf1, strlen(buf1));
}


int main() {
    char command[1024];
    char command_cpy[1024];
    char command_last[1024];
    int status_last;
    char *token;
    char *outfile;
    int i, fd, append = 0, amper = 0, piping = 0, redirection = 0, redirect_STD_OUT = 0, redirect_STD_ERR = 0, retid = 0, status = 0, argc1 = 0;
    char prompt[1024] = "hello";
    int fildes[2];
    char *argv1[10];
    char *argv2[10];

    // Assign a signal handler
    signal(SIGINT, sig_handler);

    while (1) {
        printf("===========================================================\n");
        printf("piping(|) | amper(&) | redirection(>) |     command      | \n");
        printf("===========================================================\n");
        printf("   %d      |    %d    |       %d        |        %s        | \n", piping, amper, redirect_STD_ERR || redirect_STD_OUT, command);
        printf("===========================================================\n");

        printf("===========================================================\n");
        printf("retid      | status   |    outfile       |      argc1        | \n");
        printf("===========================================================\n");
        printf("   %d      |    %d    |       %s        |        %d        | \n", retid, status, outfile , argc1);
        printf("===========================================================\n");

        printf("%s: ", prompt);
        fgets(command, 1024, stdin);

        command[strlen(command) - 1] = '\0';
        printf("Command: %s\n", command);
        piping = 0;
        // Store last command
        copy_into(command, command_cpy);
        /* parse command line */
        i = 0;
        token = strtok(command," ");
        while (token != NULL) {
            printf("Token: %s\n", token);
            argv1[i] = token;
            token = strtok (NULL, " ");
            i++;
            if (token && ! strcmp(token, "|")) {
                piping++;
                break;
            }
        }
        argv1[i] = NULL;
        argc1 = i;
        
        /* Is command empty */
        if (argv1[0] == NULL)
            continue;

        /* Does command contain pipe */
        if (piping) {
            i = 0;
            while (token!= NULL)
            {
                token = strtok (NULL, " ");
                argv2[i] = token;
                i++;
                if (token && ! strcmp(token, "|")) {
                    piping++;
                    break;
                }
            }
            
            argv2[i] = NULL;
        }
        if (piping > 1) {
            printf("Handle multiple piping commands:\n%s\n", command_cpy);
        }

        if (! strcmp(argv1[0], "quit")) {
            printf("Quitting...\n");
            fflush(stdin);
            exit(1);
        }

        /* Does command line end with & */ 
        if (! strcmp(argv1[argc1 - 1], "&")) {
            amper = 1;
            argv1[argc1 - 1] = NULL;
        }

        else 
            amper = 0; 

        /* Redirections */
        /* Write / Truncate */
        if (i > 1 && !strcmp(argv1[i - 2], ">")) {
            // printf("i = %d, argv[i - 2] = %s, \n%s\n", i, argv[i-2], command);
            redirect_STD_OUT = 1;
            argv1[i - 2] = NULL;
            outfile = argv1[i - 1];
        }
        else {
            redirect_STD_OUT = 0;
            redirection = 0;
        }

        /* Write / Append */
        if (i > 1 && !strcmp(argv1[i - 2], ">>")) {
            // printf("i = %d, argv[i - 2] = %s, \n%s\n", i, argv[i-2], command);
            redirect_STD_OUT = 1;
            append = 1;
            argv1[i - 2] = NULL;
            outfile = argv1[i - 1];
        }
        else {
            redirect_STD_OUT = 0;
            redirection = 0;
        }

        /* for commands not part of the shell command language */ 

        if (!strcmp(argv1[0], "echo") && !strcmp(argv1[1], "$?")) {
            printf("Last command exit status: %d (%s)\n", status, command_last);
            continue;
        }

        if (!strcmp(argv1[0], "!!")) {
            printf("Executing last command (%s)\n", command_last);
            memset(command, '\0', 1024);
            int size = strlen(command_cpy);
            for (int i = 0; i < size; ++i) {
                command[i] = command_cpy[i];
            }
        }

        if (argv1[i - 2] != NULL && ! strcmp(argv1[i - 2], "2>")) {
            redirect_STD_ERR = 1;
            argv1[i - 2] = NULL;
            outfile = argv1[i - 1];
            // printf("Outfile: %s\n", outfile);
        }
        else {
            redirect_STD_ERR = 0;
            redirection = 0;
        }

        
        if (argv1[i - 2] != NULL && argv1[i - 3] != NULL &&
             (!(strcmp(argv1[i - 2], "=") || strcmp(argv1[i - 3], "prompt")))) {
            // printf("Change prompt to %s!\n", argv[i - 1]);
            memset(prompt, '\0', 1024);  // Clear prompt buffer
            strncpy(prompt, argv1[i - 1], strlen(argv1[i - 1])); // Copy content into prompt buffer.
            argv1[i - 1] = NULL;
            continue;
        }
        copy_into(command_cpy, command_last);
        int child_pid = fork();
        if (child_pid == 0) {
            /* redirection of IO*/
            if (redirect_STD_OUT) {
                if (append) {
                    printf("outfile: %s\n", outfile);
                    fd = open(outfile, O_CREAT | O_APPEND | O_RDWR, 0660);
                }
                else {
                    fd = creat(outfile, 0660); 
                }
                close (STDOUT_FILENO);
                dup(fd);
                close(fd);
                /* stdout is now redirected */
            }
            else if (redirect_STD_ERR) {
                fd = creat(outfile, 0660); 
                close (STDERR_FILENO);
                dup(fd);
                close(fd);
                /* stderr is now redirected */
            }
            
        if (piping) {
            printf("Piping...\n");
            // printf("fildes: %s\n", fildes);
            pipe (fildes);
            if (fork() == 0) { 
                /* first component of command line */ 
                close(STDOUT_FILENO); 
                dup(fildes[1]); 
                close(fildes[1]); 
                close(fildes[0]); 
                /* stdout now goes to pipe */ 
                /* child process does command */ 
                execvp(argv1[0], argv1);
            } 
            /* 2nd command component of command line */ 
            close(STDIN_FILENO);
            dup(fildes[0]);
            close(fildes[0]); 
            close(fildes[1]); 
            /* standard input now comes from pipe */ 
            if (execvp(argv2[0], argv2)) {
                // exit(-2); // Error in executing command while piping
            }
        } 
        else
            if (execvp(argv1[0], argv1) == -1) {
                exit(-1); // Error in executing command
            }
        } else if (child_pid < 0) {
            printf("fork() failed!\n");
        }
        /* parent continues here */
        if (amper == 0) {
            retid = wait(&status);
            // printf("retid: %d\n", retid);
            // printf("status: %d\n", status);
        }
    }
}
