#include <iostream>
#include "list.hpp"
#include <list>

int main()
{
	std::list<int> list{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	list.insert(++list.begin(), ++++list.begin(), list.end());
	
	for (auto&& el : list) {
		std::cout << el << ' ';
	}
	std::cout << '\n' << list.size() << '\n';
	return 0;
}