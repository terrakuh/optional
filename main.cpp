#include <iostream>
#include <iostream>

#include "optional.hpp"

inline void hello(optional<const char*> _what)
{
	_what.if_present([](auto x) { std::cout << "Hello, " << x << std::endl; });
}

inline optional<std::string> map()
{
	return "";
}

int main()
{
	hello({});
	hello("");

	auto a = []() {return 3.4; };


	optional_detail::function_result<decltype(a)>::type;


	try {
		optional<int> _l;

		_l.if_present([](auto x) { std::cout << x << std::endl; });

		std::cout << _l.or_else(55) << std::endl;

		std::cout << _l.map([](auto x) { return static_cast<double>(x) + 0.6; }).get() << std::endl;

	} catch (const no_such_element_error& e) {
		std::cout << e.what() << std::endl;
	}

	system("pause");
}