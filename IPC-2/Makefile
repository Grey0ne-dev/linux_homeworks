prime_calculator: main.o Eratosthenes.o
	g++ -o prime_calculator main.o Eratosthenes.o

main.o: main.cpp Eratosthenes.h
	g++ -c main.cpp

InteractiveShell.o: Eratosthebes.cpp Eratosthenes.h
	g++ -c Eratosthenes.cpp

clean:
	rm -f *.o prime_calculator

