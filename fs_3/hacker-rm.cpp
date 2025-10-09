#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

void verify(int i, const char* msg) {
    if (i < 0) {
        perror(msg);
        exit(errno);
    }
}

void shred(int fd, size_t size) {
    char *garbagebuffer = (char*)malloc(size);
    if (!garbagebuffer) {
        perror("malloc");
        exit(1);
    }

    for (size_t i = 0; i < size; ++i)
        garbagebuffer[i] = '0';

    lseek(fd, 0, SEEK_SET);
    write(fd, garbagebuffer, size);

    free(garbagebuffer);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    char *filepath = argv[1];
    int fd = open(filepath, O_RDWR);
    verify(fd, "open");

    off_t size = lseek(fd, 0, SEEK_END);
    verify(size, "lseek");

    printf("Shredding %s (%ld bytes)\n", filepath, size);

    for (int i = 0; i < 3; ++i)
        shred(fd, size);

    close(fd);
    unlink(filepath);

    printf("File shredded and deleted.\n");
    return 0;
}

