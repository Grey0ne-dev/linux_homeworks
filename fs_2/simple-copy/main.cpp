#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

void verify(int i) {
	if(i == -1) {
		printf("not correct source or destination folder\n");
		exit(1);
	}
}



int main (int argc, char* argv[]) {

	const size_t BUFFER = 16;
	
	if(argc < 3) {
		printf("source and destination files are not provided\n");
		exit(1);
	}
	
	const char * src = argv[1];
	const char * dest = argv[2];
	int srcfd = open(src, O_RDONLY);
	verify(srcfd);
	int destfd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
	verify(destfd);
	char * buf = (char*)malloc(BUFFER);
	size_t count = 0;
	size_t readCount = 0;
	while(1) {
		readCount = read(srcfd, buf, BUFFER);
		verify(readCount);
		if(!readCount) {break;}
		count += readCount;
		size_t writeCount = write(destfd, buf, readCount);
		verify(writeCount);
	}


		close(srcfd);
		close(destfd);
		free(buf);
		printf("complete\n");
	return 0;
}
