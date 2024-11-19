#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define FILENAME "files2/shared_file.txt"
#define LOCKFILE "files2/file.lock"

void write_to_file(const char *text) {
    FILE *file = fopen(FILENAME, "w");
    if (file == NULL) {
        perror("Failed to open file");
        exit(1);
    }
    fprintf(file, "%s\n", text);
    fclose(file);
}

void read_from_file() {
    char buffer[256];
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(1);
    }
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("Read: %s", buffer);
    }
    fclose(file);
}

void lock() {
    while (access(LOCKFILE, F_OK) == 0) {
        usleep(50000);  // 50 ms delay to prevent busy waiting
    }
    close(open(LOCKFILE, O_CREAT, 0644));  // Create lock file
}

void unlock() {
    unlink(LOCKFILE);  // Remove lock file
}

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        for (int i = 0; i < 20; i++) {
            printf("Inside Child Loop cycle %d : \n", i+1);
            lock();
            write_to_file("Child writing to file");
            unlock();
            sleep(1);
            lock();
            read_from_file();
            unlock();
            sleep(1);
        }
    } else {
        // Parent process
        for (int i = 0; i < 20; i++) {
            printf("Inside Parent Loop cycle %d : \n", i+1);
            lock();
            write_to_file("Parent writing to file");
            unlock();
            sleep(1);
            lock();
            read_from_file();
            unlock();
            sleep(1);
        }
        wait(NULL);
    }

    return 0;
}
