#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define FILENAME "files2/shared_file.txt"

void write_to_file(const char *text, int fd) {
    lseek(fd, 0, SEEK_SET);  // Move to start of file
    ftruncate(fd, 0);        // Clear file contents
    write(fd, text, sizeof(text));
}

void read_from_file(int fd) {
    char buffer[256];
    lseek(fd, 0, SEEK_SET);  // Move to start of file
    int n = read(fd, buffer, sizeof(buffer) - 1);
    buffer[n] = '\0';
    printf("Read: %s\n", buffer);
}

void lock_file(int fd, int type) {
    struct flock fl;
    fl.l_type = type;     // F_WRLCK for writing or F_RDLCK for reading
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;         // Lock entire file
    fcntl(fd, F_SETLKW, &fl);
}

void unlock_file(int fd) {
    struct flock fl;
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fcntl(fd, F_SETLK, &fl);
}

int main() {
    int fd = open(FILENAME, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("Failed to open file");
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        for (int i = 0; i < 20; i++) {
            lock_file(fd, F_WRLCK);
            write_to_file("Child writing to file", fd);
            unlock_file(fd);
            sleep(1);
            lock_file(fd, F_RDLCK);
            read_from_file(fd);
            unlock_file(fd);
            sleep(1);
        }
    } else {
        // Parent process
        for (int i = 0; i < 20; i++) {
            lock_file(fd, F_WRLCK);
            write_to_file("Parent writing to file", fd);
            unlock_file(fd);
            sleep(1);
            lock_file(fd, F_RDLCK);
            read_from_file(fd);
            unlock_file(fd);
            sleep(1);
        }
        wait(NULL);
    }

    close(fd);
    return 0;
}
