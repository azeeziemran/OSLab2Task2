#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/file.h>  // for flock

#define FILE_NAME "shared.txt"
#define STATE_FILE "state.txt"

void set_state(const char *state) {
    int fd = open(STATE_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    write(fd, state, strlen(state));
    close(fd);
}

void get_state(char *buffer, size_t size) {
    int fd = open(STATE_FILE, O_RDONLY);
    if (fd < 0) {
        strcpy(buffer, "INIT");
        return;
    }
    int n = read(fd, buffer, size - 1);
    if (n > 0) buffer[n] = '\0';
    else buffer[0] = '\0';
    close(fd);
}

void write_file(const char *msg, const char *who) {
    int fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("write open");
        return;
    }

    flock(fd, LOCK_EX);  // Lock the file
    write(fd, msg, strlen(msg));
    flock(fd, LOCK_UN);  // Unlock
    close(fd);

    printf("Write: %s wrote\n", who);
}

void read_file(const char *who) {
    char buffer[256] = {0};
    int fd = open(FILE_NAME, O_RDONLY);
    if (fd < 0) {
        perror("read fopen");
        return;
    }

    flock(fd, LOCK_EX);  // Lock for reading too (flock doesn't have read-locks)
    int n = read(fd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("Read: %s read\n", who);
    }
    flock(fd, LOCK_UN);
    close(fd);
}

void erase_file() {
    int fd = open(FILE_NAME, O_WRONLY | O_TRUNC);
    if (fd >= 0) {
        flock(fd, LOCK_EX);
        flock(fd, LOCK_UN);
        close(fd);
        printf("File erased\n");
    } else {
        perror("erase open");
    }
}

int main() {
    set_state("A_WRITE");  // Initial state

    pid_t pid = fork();

    if (pid == 0) {
        // Process B
        while (1) {
            char state[20] = {0};
            get_state(state, sizeof(state));

            if (strcmp(state, "B_READ") == 0) {
                read_file("B");
                set_state("A_DELETE");
            } else if (strcmp(state, "B_WRITE") == 0) {
                write_file("Message from B\n", "B");
                set_state("A_READ");
            } else if (strcmp(state, "B_DELETE") == 0) {
                erase_file();
                set_state("A_WRITE");
            } else {
                sleep(1);
            }
        }
    } else if (pid > 0) {
        // Process A
        while (1) {
            char state[20] = {0};
            get_state(state, sizeof(state));

            if (strcmp(state, "A_WRITE") == 0) {
                write_file("Message from A\n", "A");
                set_state("B_READ");
            } else if (strcmp(state, "A_READ") == 0) {
                read_file("A");
                set_state("B_DELETE");
            } else if (strcmp(state, "A_DELETE") == 0) {
                erase_file();
                set_state("B_WRITE");
            } else {
                sleep(1);
            }
        }
    } else {
        perror("fork failed");
        return 1;
    }

    return 0;
}
