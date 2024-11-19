#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


#define FILENAME "files/shareds_file.txt"

void write_to_file(const char *text, int fd) {
    lseek(fd, 0, SEEK_SET);
    write(fd, text, strlen(text));
    fsync(fd);
}

void read_from_file(int fd, int a) {
    if(a == 0){
        printf("Read By Parent: \n");
    }else{
        printf("Read By Child: \n");
    }
    char buffer[256] = {0};
    lseek(fd, 0, SEEK_SET);
    read(fd, buffer, 255);
    printf("Read: %s\n", buffer);
}

void acquire_lock(int fd, struct flock *lock) {
    lock->l_type = F_WRLCK;
    while (fcntl(fd, F_SETLK, lock) == -1) {
        usleep(1000); // Wait for lock
    }
}

void release_lock(int fd, struct flock *lock) {
    lock->l_type = F_UNLCK;
    fcntl(fd, F_SETLK, lock);
}

int main() {
    remove(FILENAME);
    int fd = open(FILENAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    pid_t pid = fork();

    struct flock lock;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (pid == 0) { // Child process
        while (1) {
            printf("Inside Child Loop: \n");
            acquire_lock(fd, &lock);
            write_to_file("Child process writing...", fd);
            sleep(1);
            read_from_file(fd,1);
            release_lock(fd, &lock);
        }
    } else { // Parent process
        while (1) {
            printf("Inside Parent Loop: \n");
            acquire_lock(fd, &lock);
            write_to_file("Parent process writing...", fd);
            sleep(1);
            read_from_file(fd, 0);
            release_lock(fd, &lock);
        }
    }

    close(fd);
    return 0;
}