#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include "signal.h"
#include <string.h>

#define SIZE 1024
#define MAXVARS 200

struct var {
    char* str;
    int global;
};

static struct var tab[MAXVARS];
int env_size = 0;

/**
 * 
 * */
static struct var* find_item(char *name, int first_blank){
    int i;
    int len = strlen(name);
    char *s;
    for(i=0; i<MAXVARS && tab[i].str != NULL; i++){
        s = tab[i].str;
        if ((strncmp(s, name, len) == 0) && (s[len] == '=')){
            return &tab[i];
        }
    }
    if(i < MAXVARS && first_blank)
        return &tab[i];
    return NULL;
}

char *new_string(char *name, char *val){
    char *retval;
    retval = malloc(strlen(name) + strlen(val) + 2);
    if(retval != NULL)
        sprintf(retval, "%s=%s",  name, val);
    return retval;
}

int VLstore(char *name, char *val){
    struct var *itemp;
    char *s;
    int rv = 1;
    
    if((itemp = find_item(name,1)) != NULL && (s = new_string(name, val)) != NULL){
        if (itemp->str) {
            free(itemp->str);
        } else {
            env_size++;
        }
        itemp->str = s;
        rv = 0;
    }
    return rv;
}

char *VLookup(char *name){
    struct var *itemp;
    if((itemp = find_item(name, 0)) != NULL)
        return itemp-> str + 1 + strlen(name);
    return "";
}

int VLexport(char *name){
    struct var *itemp;
    int rv = 1;
    if((itemp = find_item(name, 0)) != NULL){
        itemp->global = 1;
        rv = 0;
    }
    else if(!strcmp(VLookup(name), ""))
        rv = VLexport(name);
    return rv;
}

int VLenviron2table(char *env[]){
    int i;
    char *newstring;
    for (i=0; env[i] != NULL; i++){
        if(i == MAXVARS)
            return 0;
        newstring = malloc(1 + strlen(env[i]));
        if(newstring == NULL)
            return 0;
        strcpy(newstring, env[i]);
        tab[i].str = newstring;
        tab[i].global = 1;
        env_size++;
    }
    while (i < MAXVARS){
        tab[i].str = NULL;
        tab[i++].global = 0;
    }
    return 1;
}
char **VLtable2environ(){
    int i, j ,n = 0;
    char **envtab;
    for(i = 0; MAXVARS && tab[i].str != NULL; i++){
        if(tab[i].global == 1)
            n++;
        envtab = (char **) malloc((n+1) * sizeof(char));
       if(envtab == NULL)
            return NULL;
        for(i = 0, j = 0; i<MAXVARS && tab[i].str != NULL; i++){
            if (tab[i].global == 1)
            envtab[j++] = tab[i].str;
        }
        envtab[j] = NULL;
        return envtab;
    }
}

void setup(){
    extern char **environ;
    VLenviron2table(environ);
    signal (SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
}

void sig_handler(int sig) {
    printf("You've pressed Ctrl+C!\n");
}

void copy_into(char* buf1, char* buf2) {
    memset(buf2, '\0', 1024);
    strncpy(buf2, buf1, strlen(buf1));
}

int valid_name(char *str) {
    return 1;
}

int valid_path(char *path) {
    int retid, status;
    if (fork() == 0) {
        char* args[3] = {"./stree", path};
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        
        if (execvp("./stree", args) == -1) {
            // printf("execvp(./stree) failed!\n");
        }
    }
    // printf("Waiting...\n");
    retid = wait(&status);
    if (status == 65280) {
        // File path was not found using our `stree` tool
        // printf("No such directory!\n");
        return 0;
    }
    else if (status >= 0) { 
        // File path is valid
        // printf("status = %d\n", status);
        // printf("./stree returned succssfully!\n");
        return 1;
    }
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

    setup();
    
    // char *argv[10];

    // argv = (char*)malloc(10);

    char *argv1[10];
    char *argv2[10];
    char *pwd;  // pwd

    char buf[SIZE];
    pwd = getcwd(buf, SIZE); // pwd
    int size = strlen(pwd);
    // char pwd2[size+1];       // pwd/
    // memset(pwd2, '\0', size);
    // strncat(pwd2, pwd, size+1);
    // strncat(pwd2, seperator, strlen(seperator));

    
    // Assign a signal handler
    signal(SIGINT, sig_handler);
    // signal(, sig_handler);

    while (1) {

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
            // printf("Token: %s\n", token);
            argv1[i] = token;
            if (!strcmp(argv1[0], "!!")) {
            printf("%s\n", command_last);
            memset(command, '\0', 1024);
            int size = strlen(command_cpy);
            for (int i = 0; i < size; ++i) {
                command[i] = command_cpy[i];
            }
        }
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

        if (! strcmp(argv1[0], "pwd")) {
            printf("%s\n", pwd);
            continue;
        }

        if (! strcmp(argv1[0], "cd")) {
            // printf("Change pwd to: %s/%s\n", pwd, argv1[1]);
            char dir1[SIZE];
            char dir2[SIZE];
            memset(dir1, '\0', SIZE);
            memset(dir2, '\0', SIZE);
            strncat(dir1, pwd, strlen(pwd));
            dir1[strlen(pwd)] = '/';
            strncat(dir1 + strlen(pwd), argv1[1], strlen(argv1[1]));

            // strncat(dir2, pwd2, strlen(pwd2));
            // strncat(dir2 + strlen(pwd2), argv1[1], strlen(argv1[1]));
            // printf("dir1: %s\ndir2: %s\n", dir1, dir2);
            int change_dir = 0;
            if (valid_path(dir1)) {
                // printf("Yay dir1 is a valid path! %s\n", dir1);
                sprintf(pwd, "%s", dir1);
                change_dir = 1;
                // printf("new pwd: %s\n", pwd);

            } else if (valid_path(dir2)) {
                // printf("Yay! dir2 is a valid path: %s\n", dir2);
                sprintf(pwd, "%s", dir2);
                change_dir = 1;
                // printf("new pwd: %s\n", pwd);

            } else if (valid_path(argv1[1])) {
                // printf("Yay argv[1] is a valid path!\n");
                sprintf(pwd, "%s", argv1[1]);
                change_dir = 1;
                // printf("New pwd: %s\n", pwd);
            }
            else {
                printf("Path file is invalid!\n");
            }
            if (change_dir) {
                if (chdir(pwd) == -1) {
                    perror("chdir()");
                }
            }
            continue;
        }

        if (! strcmp(argv1[0], "quit")) {
            printf("Quitting...\n");
            extern char ** environ;
            environ = VLtable2environ();
            // fflush(stdin);
            exit(0);
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
        if (i > 1 && argv1[i - 2] != NULL && !strcmp(argv1[i - 2], ">")) {
            // printf("i = %d, argv[i - 2] = %s, \n%s\n", i, argv[i-2], command);
            redirect_STD_OUT = 1;
            argv1[i - 2] = NULL;
            char *t;
            outfile = argv1[i - 1];
        }
        else {
            redirect_STD_OUT = 0;
            redirection = 0;
        }

        /* Write / Append */
        if (i > 1 && argv1[i - 2] != NULL && !strcmp(argv1[i - 2], ">>")) {
            // printf("i = %d, argv[i - 2] = %s, \n%s\n", i, argv[i-2], command);
            redirect_STD_OUT = 1;
            append = 1;
            argv1[i - 2] = NULL;
            outfile = argv1[i - 1];
        }
        else {
            append = 0;
            redirection = 0;
        }

        /* for commands not part of the shell command language */ 

        if (!strcmp(argv1[0], "echo")) {
            if (!strcmp(argv1[1], "$?")) {
                printf("Last command exit status: %d (%s)\n", status, command_last);
                continue;
            }

            char *var = strchr(argv1[1], '$');
            if (var && var == argv1[1]) {
                /* Print variables */
                printf("%s\n", VLookup(var + 1));
                continue;
            }
        }

        if (argv1[0][0] == '$' && argc1 == 3) {
            if (!strcmp(argv1[1], "=") ) {
                char *var_name = strchr(argv1[0], '$');
                if (valid_name(var_name + 1) && argv1[2] != NULL) {
                    // Store key=value
                    VLstore(var_name + 1, argv1[2]);
                    VLexport(var_name + 1);
                }
            }
            continue;
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

        if (argc1 == 3 && argv1[i - 2] != NULL && argv1[i - 3] != NULL &&
             (!(strcmp(argv1[i - 2], "=") || strcmp(argv1[i - 3], "prompt")))) {
            // printf("Change prompt to %s!\n", argv[i - 1]);
            memset(prompt, '\0', 1024);  // Clear prompt buffer
            strncpy(prompt, argv1[i - 1], strlen(argv1[i - 1])); // Copy content into prompt buffer.
            argv1[i - 1] = NULL;
            continue;
        }
        // Save last command, which is about to be executed
        copy_into(command_cpy, command_last);
        // int p = 0;
        // while (argv1[p]) {
        //     p++;
        // }
        // char *t = malloc(strlen(pwd) + 2);
        // sprintf(t, "%s/", pwd);
        // if (t)
        //     argv1[p] = t;

        // printf("argv1[p] = %s\n", argv1[p]);
        int second_proc_pid = fork();
        if (second_proc_pid == 0) {
            // ls -l | *grep 26*
            /* redirection of IO*/
            if (redirect_STD_OUT == 1) {
                if (append) {
                    // printf("outfile: %s\n", outfile);
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
                // printf("Piping...\n");
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
                        // printf("fildes[0]: %d\n", fildes[0]);
                        // printf("fildes[1]: %d\n", fildes[1]);
                        close(fildes[1]);
                        close(fildes[0]);
                        /* stdout now goes to pipe */ 
                        /* child process does command */ 
                        if (execvp(argv1[0], argv1) == -1) {
                            printf("Failed to execute %s\n", argv1[0]);
                            exit(-1);
                        }
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
            if (execvp(argv1[0], argv1) == -1) {
                printf("Failed to execute %s\n", argv1[0]);
                exit(-1);
            }
            // execvp(argv1[0], argv1);
        }
        /* parent continues here */ // &
        if (amper == 0) {
            retid = wait(&status);
        }
    }
    extern char ** environ;
    environ = VLtable2environ();
}
