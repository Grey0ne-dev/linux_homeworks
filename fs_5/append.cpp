#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
int main (int argc, char ** argv) {


	char firstline[] = "first line\n";
	char secondline[] = "second line\n";
	const char * filepath = argv[1];
	int fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0644); 
	if(fd < 0) {
		std::cerr << "invalid filepath\n";
		exit(1);
	}
	int fd2 = dup(fd);
	if(fd2 < 0) {
		std::cerr << "couldnt duplicate file descriptor\n";
		exit(1);
	}
	write(fd, firstline, strlen(firstline));
    if(close(fd) < 0) {
		std::cerr << "error closing file descriptor\n";
		exit(1);
	}
	write(fd2, secondline, strlen(secondline));
	if(close(fd2) < 0) {
		std::cerr << "error closing file descriptor\n";
		exit(1);
	}
	return 0;
}
