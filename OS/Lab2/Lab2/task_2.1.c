#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Function for child process to repeatedly write a character
void child_process(const char *char_to_write) {
    while (1) {
        fputc(*char_to_write, stdout);  // Write character to stdout
        fflush(stdout);                // Flush output to ensure immediate display
        //usleep(1000);                  // Add a 1 ms delay
    }
}

int main() {
    pid_t pid1, pid2;

    // Create the first child process
    pid1 = fork();
    if (pid1 == 0) {
        child_process("A");
        exit(0);
    }

    // Create the second child process
    pid2 = fork();
    if (pid2 == 0) {
        child_process("_");
        exit(0);
    }

    // Parent process waits for the child processes
    wait(NULL);
    wait(NULL);

    return 0;
}

