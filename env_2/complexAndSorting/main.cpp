#include "sort.h"
#include "complex.h"
#include <iostream>
#include <vector>

int main () {

	std::vector<int> vec = {1, 3, -1, -5};
	std::vector<Complex> vec2;
	vec2.emplace_back(Complex(1,1));
	vec2.emplace_back(Complex(8,99));
	vec2.emplace_back(Complex(0,1));
	sort(vec);
	sort(vec2);
	std::cout << "sorted vector: ";
	int n = vec.size();
	int n2 = vec2.size();
	for(int i = 0; i < n; ++i) {
		std::cout<<vec[i] << ' ';
	}
	std::cout << '\n';
	for(int i = 0;  i < n2;++i){
		std::cout<<vec2[i] <<  ' ';
	}
	std::cout << '\n';
	Complex i(0,1);
	Complex minusone = i*i;

	std::cout << "i = " << minusone << std::endl;

return 0;
}
