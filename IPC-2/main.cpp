#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <csignal>
#include <cstdlib>
#include "Eratosthenes.h"

int fd1[2], fd2[2]; // made global to let handler acess them

void check(int errnum, const char* errmsg) {
    if(errnum < 0) {
        std::cerr << errmsg << " failed\n";
        raise(SIGINT);
    }
}

void sighandler(int signum) {
    std::cerr << "\nSignal " << signum << " received. Closing pipes...\n";
    close(fd1[0]);
    close(fd1[1]);
    close(fd2[0]);
    close(fd2[1]); //no error hanling for a reason
    exit(1);
}

int main() {
    // setuping sigactions
    int status;
    struct sigaction sa;
    sa.sa_handler = sighandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    status = sigaction(SIGINT, &sa, nullptr);
    check(status, "sigaction");
    status = sigaction(SIGTERM, &sa, nullptr);
    check(status, "sihaction");
    check(pipe(fd1), "pipe1"); // p -> c
    check(pipe(fd2), "pipe2"); // c -> p

    int pid = fork();
    check(pid, "fork");

    if(pid == 0) { // if child
        status = close(fd1[1]);
	check(status, "close");
        status = close(fd2[0]);
	check(status, "close");

        Eratosthenes sieve(1000000); // big sieve )

        while(true) {
            int n;
            ssize_t s = read(fd1[0], &n, sizeof(n));
            if(s <= 0) break;

            const auto& primes = sieve.getPrimes();
            int result = (n >= 1 && n <= (int)primes.size()) ? primes[n-1] : -1;

            status = write(fd2[1], &result, sizeof(result));
	    check(status, "write fd2");
        }

        status = close(fd1[0]);
	check(status, "close");
        status = close(fd2[1]);
	check(status, "close");
        exit(0);

    } else { // if parent
        status = close(fd1[0]);
	check(status, "close");
        status = close(fd2[1]);
	check(status, "close");

        while(true) {
            std::cout << "[Parent] Enter number (n-th prime) or 0 to quit: ";
            int n;
            std::cin >> n;
            if(n == 0) break;

            status = write(fd1[1], &n, sizeof(n));
	    check(status, "write");

            int result;
            ssize_t s = read(fd2[0], &result, sizeof(result));
            if(s <= 0) {
                std::cerr << "Child closed the pipe\n";
                break;
            }

            if(result == -1)
                std::cout << "Invalid n, out of range\n";
            else
                std::cout << "The " << n << "-th prime is: " << result << "\n";
        }

        status = close(fd1[1]);
	check(status, "close");
        status = close(fd2[0]);
	check(status, "close");
        wait(nullptr);
    }

    return 0;
}

