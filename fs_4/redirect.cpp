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

    // Открываем файл только для чтения
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    // Перенаправляем стандартный ввод (stdin = 0) в этот файл
    dup2(fd, STDIN_FILENO);
    close(fd);
}

int main(int argc, char** argv)
{
    initialize(argc, argv);

    // читаем строку уже не с клавиатуры, а из файла
    std::string input;
    std::cin >> input;

    // переворачиваем строку
    std::string reversed = input;
    std::reverse(reversed.begin(), reversed.end());

    // выводим результат в стандартный поток вывода (терминал)
    std::cout << reversed << std::endl;

    return 0;
}

