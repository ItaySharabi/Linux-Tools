#include <stdio.h>
#include <ftw.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    printf("Hello, world!\n");

    char *args[] = {"./ftw2", argv[1], NULL};
    int execute = execv("./ftw2", args);
    return 0;
}