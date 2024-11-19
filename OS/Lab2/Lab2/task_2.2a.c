#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define FILENAME "files/shareds_file.txt"
#define LOCKFILE "files/files.lock"

// Function to write to the shared file
void write_to_file(const char *text) {
    FILE *file = fopen(FILENAME, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        exit(1);
    }
    fprintf(file, "%s", text);
    fclose(file);
}

// Function to read from the shared file
void read_from_file(int a) {
    if(a == 0){
        printf("Read By Parent: \n");
    }else{
        printf("Read By Child: \n");
    }
    char buffer[256];
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        exit(1);
    }
    fread(buffer, sizeof(char), 255, file);
    buffer[255] = '\0';
   
printf("Read: %s\n", buffer);
    fclose(file);
}

// Locking mechanism using a lock file
void lock_file() {
    while (access(LOCKFILE, F_OK) == 0) {
        usleep(1000); // Wait until lock is released
    }
    FILE *lock = fopen(LOCKFILE, "w");
    fclose(lock);
}

// Unlocking mechanism
void unlock_file() {
    unlink(LOCKFILE); // Remove the lock file
}

int main() {
    remove(FILENAME);
    remove(LOCKFILE);
    int swap = 0;

    pid_t pid = fork();

    if (pid == 0) { // Child process
        while (1) {
            printf("Inside Child Loop: \n");
            lock_file();
            write_to_file("Child process writing... ");
            sleep(2);
            read_from_file(1);
            unlink(FILENAME); // Erase the file
            unlock_file();
        }
    } else { // Parent process
        while (1) {
            printf("Inside Parent Loop: \n");
            lock_file();
            write_to_file("Parent process writing... ");
            sleep(2);
            read_from_file(0);
            unlink(FILENAME); // Erase the file
            unlock_file();
        }
    }

    return 0;
}
