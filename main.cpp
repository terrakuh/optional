#include "optional.hpp"

#include <iostream>
#include <limits>
#include <string>

inline void hello(optional<std::string> what)
{
    what.if_present([](const std::string& x) { std::cout << "Hello, " << x << std::endl; });
}

inline double convert(optional<std::string> what)
{
    return what.map(&std::stod, static_cast<size_t*>(0)).or_else(std::numeric_limits<double>::quiet_NaN());
}

int main()
{
    hello({});
    hello("World!");

    std::cout << convert("5.36") << std::endl;
    std::cout << convert({}) << std::endl;

    std::cout << "c str: " << optional<std::string>("hi").map(&std::string::c_str).or_else(nullptr) << std::endl;

    optional<int> l;

    std::cout << "l or 55: " << l.or_else(55) << std::endl;
    std::cout << "l or 3*3: " << l.or_else_get([] { return 3 * 3; }) << std::endl;

    // hash of empty optional
    std::cout << "hash of empty optional==0: " << (std::hash<optional<int>>()(l) == std::hash<int>()(0)) << std::endl;

    l.if_present_or_else([](int x) { std::cout << "l is present: " << x << std::endl; },
                         [&l] {
                             std::cout << "l is not present; setting it to 61" << std::endl;
                             l.emplace(61);
                         });

    std::cout << "l as double + 10%: " << l.map([](int x) { return x * 1.1; }).get() << std::endl;

    try {
        optional<int> x;

        // getting empty optional
        x.get();
    } catch (const no_such_element_error& e) {
        std::cout << e.what() << std::endl;
    }
}