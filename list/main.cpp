#include <iostream>
#include "list.hpp"

int main()
{
	std::allocator<char> alloc;
	my_lib::list<int> list(alloc);
	my_lib::list<int> list2(list);
	return 0;
}