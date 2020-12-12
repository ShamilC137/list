#pragma once
#ifndef MY_UTILITIES_HPP
#define MY_UTILITIES_HPP

namespace my_lib {
	template <class iter, class = void>
	struct is_iterator
	{
		constexpr static bool value{ false };
	};

	template <class iter>
	struct is_iterator<iter,
		std::void_t<typename iter::iterator_category,
		typename iter::value_type,
		typename iter::pointer,
		typename iter::reference,
		typename iter::difference_type>
	>
	{
		constexpr static bool value{ true };
	};
}
#endif