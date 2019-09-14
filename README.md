# optional
C++11 compliant, [Java-like](https://docs.oracle.com/javase/8/docs/api/java/util/Optional.html), single-header and public domain optional utility

# Installation
Just copy the `optional.hpp` file into your project folder and that's it!

# Example
```cpp
#include <iostream>
#include <string>
#include <limits>

#include "optional.hpp"

inline void hello(optional<std::string> _what)
{
  _what.if_present([](auto x) { std::cout << "Hello, " << x << std::endl; });
}

inline double convert(optional<std::string> _what)
{
  return _what.map([](auto x) { return std::stod(x); })
              .or_else(std::numeric_limits<double>::quiet_NaN());
}

int main()
{
  hello({});
  hello("World!");
  
  convert("5.66");
  convert({});
}

```
