#include "Eratosthenes.h"

Eratosthenes::Eratosthenes(int n) : n(n), is_prime(n + 1, true) {
    if(n >= 0) is_prime[0] = false;
    if(n >= 1) is_prime[1] = false;

    for(int i = 2; i * i <= n; i++) {
        if(is_prime[i]) {
            for(int j = i * i; j <= n; j += i)
                is_prime[j] = false;
        }
    }

    for(int i = 2; i <= n; i++)
        if(is_prime[i])
            primes.push_back(i);
}

bool Eratosthenes::isPrime(int x) const {
    if(x < 0 || x > n) return false;
    return is_prime[x];
}

const std::vector<int>& Eratosthenes::getPrimes() const {
    return primes;
}

