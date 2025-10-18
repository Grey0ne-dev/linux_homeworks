shell: main.o InteractiveShell.o
	g++ -o shell main.o InteractiveShell.o

main.o: main.cpp InteractiveShell.h
	g++ -c main.cpp

InteractiveShell.o: InteractiveShell.cpp InteractiveShell.h
	g++ -c InteractiveShell.cpp

clean:
	rm -f main.o InteractiveShell.o shell

