#include <iostream>
#include <list>
#include "list.hpp"
#include <initializer_list>
#include <vld.h>

// Test structure
struct A {
	int a;
	A() noexcept :a{}
	{
		std::cout << "A()\n";
	}

	A(int b) noexcept : a(b)
	{
		std::cout << "A(int)\n";
	}

	A(const A& b) noexcept :a(b.a)
	{
		std::cout << "A(const A&)\n";
	}

	A(A&& a) noexcept : a(std::move(a.a))
	{
		std::cout << "A(A&&)\n";
	}

	A& operator =(const A& b) noexcept
	{
		a = b.a;
		std::cout << "operator=(const A&)\n";
		return *this;
	}

	A& operator =(A&& b) noexcept
	{
		a = b.a;
		std::cout << "operator=(A&&)\n";
		return *this;
	}

	~A() noexcept
	{
		std::cout << "~A()\n";
	}

	bool operator<(const A& b) const noexcept
	{
		std::cout << "operator<(const A&)\n";
		return a < b.a;
	}

	bool operator>(const A& b) const noexcept
	{
		std::cout << "operator>(const A&)\n";
		return a > b.a;
	}

	bool operator==(const A& b) const noexcept
	{
		std::cout << "operator==(const A&)\n";
		return a == b.a;
	}

	operator int() noexcept
	{
		std::cout << "operator int()\n";
		return a;
	}

	friend std::ostream& operator<<(std::ostream& stream, const A& a)
	{
		stream << a.a;
		return stream;
	}

	friend std::istream& operator>>(std::istream& stream, A& a)
	{
		stream >> a.a;
		return stream;
	}
};

int main()
{
	my_lib::list<A> a{ 1, 3, 5, 7 };
	my_lib::list<A> b{ 1,  2, 4 };
	auto before = a.begin();
	a.merge(b);
	auto after = a.begin();
	std::cout << std::boolalpha << (before == after);
	return 0;
}