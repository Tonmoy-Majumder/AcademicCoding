#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define FILENAME "shared_file.txt"
#define LOCKFILE "lockfile.lock"

void write_to_file(const char *text, const int process) {
    printf("Writing by Process: %d \n",process);
    int fd = open(FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        printf("In process: %d ",process);
        perror("Error opening file for writing \n");
        exit(1);
    }
    write(fd, text, strlen(text));
    close(fd);
}

void read_from_file(const int process) {
    printf("READING by Process: %d \n",process);
    char buffer[100];
    int fd = open(FILENAME, O_RDONLY);
    if (fd < 0) {
        printf("In process: %d ",process);
        perror("Error opening file for reading \n");
        exit(1);
    }
    read(fd, buffer, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';  // Ensure null termination
    printf("Read from file: %s\n", buffer);
    close(fd);
}

void delete_file(const int process) {
    printf("DeLeTiNg by Process: %d \n",process);
    if (remove(FILENAME) != 0) {
        printf("In process: %d ",process);
        perror("Error deleting the file \n");
        exit(1);
    }
}

void lock_file(int lock_fd, const int process) {
    printf("Locking by Process: %d \n",process);
    struct flock lock;
    lock.l_type = F_WRLCK;   // Write lock
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if (fcntl(lock_fd, F_SETLKW, &lock) == -1) {
        perror("Failed to lock file \n");
        exit(1);
    }
}

void unlock_file(int lock_fd, const int process) {
    printf("Un-Locking by Process: %d \n",process);
    struct flock lock;
    lock.l_type = F_UNLCK;   // Unlock
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if (fcntl(lock_fd, F_SETLKW, &lock) == -1) {
        perror("Failed to unlock file \n");
        exit(1);
    }
}

int main() {
    remove(FILENAME);
    int fd = open(FILENAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed \n");
        exit(1);
    }

    int lock_fd = open(LOCKFILE, O_CREAT | O_RDWR, 0666);
    if (lock_fd < 0) {
        perror("Failed to create lock file \n");
        exit(1);
    }

    int role = (pid == 0) ? 0 : 1;  // 0 for process A, 1 for process B

    while (1) {
        lock_file(lock_fd, role);  // Acquire lock

        if (role == 0) {  // Process A role
            printf("I am Process %d \n", role);
            write_to_file("Hello from %d", role);
            printf("Simulate processing time \n");
            sleep(1);  // Simulate processing time
            printf("Release lock by %d so B can read \n", role);
            unlock_file(lock_fd, role);  // Release lock so B can read
            printf("Wait for B to read \n");
            sleep(1);  // Wait for B to read
            printf("Re-acquire lock to delete file \n");
            lock_file(lock_fd, role);  // Re-acquire lock to delete file
            delete_file(role);
        } else {  // Process B role
            printf("I am Process %d \n", role);
            read_from_file(role);
            printf("Simulate processing time \n");
            sleep(1);  // Simulate processing time
            printf("Release lock by %d so A can read \n", role);
            unlock_file(lock_fd, role);  // Release lock so A can read
            printf("Wait for A to write \n");
            sleep(1);  // Wait for A to write
            printf("Re-acquire lock to delete file \n");
            lock_file(lock_fd, role);  // Re-acquire lock to delete file
            delete_file(role);
        }

        printf("Release lock after deletion \n");
        unlock_file(lock_fd,role);  // Release lock after deletion
        printf("Switch roles \n");
        role = 1 - role;  // Switch roles
    }
    close(lock_fd);
    return 0;
}
