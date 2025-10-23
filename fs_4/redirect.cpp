#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <algorithm>

void initialize(int argc, char** argv)
{
    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        exit(1);
    }

    // open file "READONLY"
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    // "Redirect output"
    if(dup2(fd, STDIN_FILENO) == -1) {
        perror("dup2");
        exit(1);
    }
    if(close(fd) == -1) {
        perror("close");
        exit(1);
    }
}

int main(int argc, char** argv)
{
    initialize(argc, argv);

    // reading string  from a file
    std::string input;
    std::cin >> input;

    std::string reversed = input;
    std::reverse(reversed.begin(), reversed.end());

    // result -> stdout
    std::cout << reversed << std::endl;

    return 0;
}

