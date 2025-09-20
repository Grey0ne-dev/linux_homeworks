#include <iostream>
#include <algorithm>

int main (int argc, char** argv) {
    
    for(int i = argc-1; i != 0; i--){
        std::cout << atoi(argv[i]) << ' ';
    }
    	std::cout << "\n";
    return 0;
}
