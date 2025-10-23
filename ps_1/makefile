all: do-command

do-command: main.o
	g++ main.o -o read-file

main.o: do-command.cpp


clean:
	rm *.o do-command
