#include <iostream>
#include <algorithm>

int main (int args, char ** argv) {
	if (args !=4){
	std::cerr << "Insert three numbers";
	}
    
	int a = std::atoi (argv[1]);
	int b = std::atoi (argv[2]);	
	int c = std::atoi (argv[3]);

    std::cout << "max: " << std::max(a,std::max(b,c)) << std::endl;
    std::cout << "min: " << std::min(a,std::min(b,c)) << std::endl;
    
    return 0;
    }
