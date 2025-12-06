#ifndef ERATOSTHENES_H
#define ERATOSTHENES_H

#include <vector>

class Eratosthenes {
private:
    int n;
    std::vector<bool> is_prime;
    std::vector<int> primes;

public:
    Eratosthenes(int n);
    bool isPrime(int x) const;
    const std::vector<int>& getPrimes() const;
};

#endif

