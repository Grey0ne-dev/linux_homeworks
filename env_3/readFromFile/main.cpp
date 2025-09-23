#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

void error_exit(const char *msg) {
    perror(msg);
    exit(1);
}

void myprintf(const char *str) {
    write(1, str, strlen(str));
}

int main(int argc, char **argv) {
    const size_t BUFFER_SIZE = 4096;

    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int file = open(argv[1], O_RDONLY);
    if (file == -1) {
        error_exit("open");
    }

    char *buffer = (char*)malloc(BUFFER_SIZE);
    if (!buffer) {
        close(file);
        error_exit("malloc");
    }

    ssize_t bytes_read;
    while ((bytes_read = read(file, buffer, BUFFER_SIZE)) > 0) {
        write(1, buffer, bytes_read);
    }

    if (bytes_read == -1) {
        error_exit("read");
    }

    close(file);
    free(buffer);

    myprintf("Completed reading file\n");
    return 0;
}
