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
	
    long long counter = 0;
    while (true) {
        arr.lock();
        arr[0] = static_cast<shared_array::value_type>(++counter);
        size_t idx = counter % arr.size();
        arr[idx] = static_cast<shared_array::value_type>(counter);
        std::cout << "first: wrote counter=" << counter << " at idx=" << idx << "\n";
        arr.unlock();
        usleep(500 * 1000);
    }	
    return 0;
}	
