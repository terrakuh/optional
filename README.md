# optional
C++11 compliant [Java-like](https://docs.oracle.com/javase/8/docs/api/java/util/Optional.html) optional utility

# Installation
Just copy the `optional.hpp` file into your project folder and that's it!

# Example
```cpp
#include <iostream>

#include "optional.hpp"

inline void hello(optional<const char*> _what)
{
  _what.if_present([](auto x) { std::cout << "Hello, " << x << "!\n"; });
}

int main()
{
  hello({});
  hello("World");
}

```
