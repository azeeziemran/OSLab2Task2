#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define FILE_NAME "shared.txt"
#define STATE_FILE "state.txt"

void set_lock(int fd, short type) {
    struct flock lock;
    lock.l_type = type;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd, F_SETLKW, &lock);
}

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
    read(fd, buffer, size - 1);
    close(fd);
}

void write_file(const char *msg, const char *who) {
    int fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    set_lock(fd, F_WRLCK);
    write(fd, msg, strlen(msg));
    set_lock(fd, F_UNLCK);
    close(fd);
    printf("Write: %s wrote\n", who);
}

void read_file(const char *who) {
    char buffer[256] = {0};
    int fd = open(FILE_NAME, O_RDONLY);
    set_lock(fd, F_RDLCK);
    int n = read(fd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("Read: %s read\n", who);
    }
    set_lock(fd, F_UNLCK);
    close(fd);
}

void erase_file() {
    int fd = open(FILE_NAME, O_WRONLY | O_TRUNC);
    set_lock(fd, F_WRLCK);
    set_lock(fd, F_UNLCK);
    close(fd);
    printf("File erased\n");
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
                sleep(1); // Wait a bit
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
                sleep(1); // Wait a bit
            }
        }
    } else {
        perror("fork failed");
        return 1;
    }

    return 0;
}
