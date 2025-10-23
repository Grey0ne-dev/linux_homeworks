#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
int main (int argc, char ** argv) {


	char firstline[] = "first line\n";
	char secondline[] = "second line";
	const char * filepath = argv[1];
	int fd = open(filepath, O_WRONLY); 
	if(fd < 0) {
		std::cerr << "invalid filepath\n";
		exit(1);
	}
	int fd2 = dup(fd);
	if(fd  < 0) {
		std::cerr << "couldnt duplicate file direction\n";
		exit(1);
	}
	write(fd, firstline, strlen(firstline));
	write(fd2, secondline, strlen(secondline));
	close(fd);
	close(fd2);
	return 0;



	return 0;
}
