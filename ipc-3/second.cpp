#include <iostream>
#include <chrono>
#include <unistd.h>
#include "shared_array.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <name> <size>\n";
        return 1;
    }
    std::string name = argv[1];
    size_t size = std::stoull(argv[2]);

    shared_array arr(name, size);

    while (true) {
        arr.lock();
        
        std::cout << "second: [0]=" << arr[0];
        for (size_t i = 1; i < std::min<size_t>(10, arr.size()); ++i) std::cout << ", [" << i << "]=" << arr[i];
        std::cout << "\n";
        arr.unlock();
        usleep(700 * 1000);
    }
    return 0;
}
