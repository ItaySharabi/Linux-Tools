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
    printf("test: %d\n", sig);
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

    // char *argv[10];

    // argv = (char*)malloc(10);

    char *argv1[10];
    char *argv2[10];

    
    // Assign a signal handler
    signal(SIGINT, sig_handler);
    // signal(, sig_handler);

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
        int n = 0;
        command[strlen(command) - 1] = '\0';
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
        argv1[i] = NULL; // i = 3 for [ls -l | grep 26]
        argc1 = i;


        /* Is command empty */
        if (argv1[0] == NULL)
            continue;
        

        /* Does command contain pipe */
        if (piping) {
            i = 0;
            while (token != NULL)
            {
                // ls -l | grep 26 | grep 12
                // argv2 = {"grep", "26", }
                token = strtok (NULL, " ");
                argv2[i] = token;
                i++;
                if (token && ! strcmp(token, "|")) {
                    piping++;
                    // argv2[i] = NULL;
                    // *(argv_n + (n++)) = argv2;
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
        // Save last command, which is about to be executed
        copy_into(command_cpy, command_last);
        int second_proc_pid = fork();
        if (second_proc_pid == 0) {
            // ls -l | *grep 26*
            /* redirection of IO*/
            if (redirect_STD_OUT) {
                printf("redirecting to > %s\n", outfile);
                if (append) {
                    printf("outfile: %s\n", outfile);
                    fd = open(outfile, O_CREAT | O_APPEND | O_RDWR, 0660);
                }
                else {
                    // fd = creat(outfile, 0660); 
                    fd = open(outfile, O_CREAT | O_TRUNC | O_RDWR, 0660);
                }
                close(STDOUT_FILENO);
                dup(fd); // Duplicate fd into stdout
                close(fd);
                /* stdout is now redirected */
            }
            else if (redirect_STD_ERR) {
                fd = creat(outfile, 0660); 
                close (STDERR_FILENO);
                dup(fd); // Duplicate fd into stdin
                close(fd);
                /* stderr is now redirected */
            }
            
            if (piping) {
                // i = ??
                printf("Piping...\n");
                while (piping > 0) {
                    // argv1 = something;
                    // argv2 = something;
                    printf("Piping #%d(=n)\n", n++);
                    if (pipe(fildes) == -1) {
                        perror("Pipe:");
                    }
                    int first_proc_pid = fork();
                    if (first_proc_pid == 0) {
                        /* first component of command line */ 
                        printf("first child process\n");
                        close(STDOUT_FILENO); // 1
                        /* Duplicate the output side of pipe to stdout */
                        dup(fildes[1]);
                        printf("pipeline fd duplicated to stdout!\n");
                        printf("fildes[0]: %d\n", fildes[0]);
                        printf("fildes[1]: %d\n", fildes[1]);
                        close(fildes[1]);
                        close(fildes[0]);
                        /* stdout now goes to pipe */ 
                        /* child process does command */ 
                        execvp(argv1[0], argv1);
                    } 
                    /* 2nd command component of command line */ 
                    close(STDIN_FILENO); // 0
                    /* Duplicate the input side of pipe to stdin */
                    dup(fildes[0]);
                    close(fildes[0]); 
                    close(fildes[1]); 
                    /* standard input now comes from pipe */ 
                    execvp(argv2[0], argv2);
                    piping--;
                    // Two programs finished executing 
                    // while the first redirected its output
                    // to the second
                }
            } 
            // Else, if not piping, code reaches here:
            execvp(argv1[0], argv1);
        }
        /* parent continues here */ // &
        if (amper == 0) {
            retid = wait(&status);
        }
    }
}
