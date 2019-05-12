/*
MIT License

Copyright (c) 2019 Yunus Ayar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <type_traits>
#include <utility>
#include <stdexcept>


namespace optional_detail
{

template<typename... Types>
struct make_void
{
	typedef void type;
};

template<typename... Types>
using void_t = typename make_void<Types...>::type;


template<typename Function, typename = void, typename... Arguments>
struct function_result;

template<typename Return, typename... Arguments>
struct function_result<Return(*)(Arguments...), void, Arguments...>
{
	typedef typename Return type;
};

template<typename Return, typename Class, typename... Arguments>
struct function_result<Return(Class::*)(Arguments...), void, Arguments...>
{
	typedef typename Return type;
};

template<typename Return, typename Class, typename... Arguments>
struct function_result<Return(Class::*)(Arguments...) const, void, Arguments...>
{
	typedef typename Return type;
};

template<typename Functor, typename... Arguments>
struct function_result<Functor, void_t<decltype(std::declval<Functor>()(std::declval<Arguments>()...))>, Arguments...>
{
	typedef decltype(std::declval<Functor>()(std::declval<Arguments>()...)) type;
};

template<typename Function, typename... Arguments>
using function_result_t = typename function_result<Function, void, Arguments...>::type;


class no_such_element_error : public std::runtime_error
{
public:
	no_such_element_error() noexcept : std::runtime_error("no element present in optional") {}
};


template<typename Type>
class optional
{
public:
	typedef typename Type value_type;

	optional() noexcept
	{
		_present = false;
	}
	template<typename Ty>
	optional(Ty&& _value)
	{
		new(&_data) Type(std::forward<Ty>(_value));
		_present = true;
	}
	optional(const optional& _copy)
	{
		if (_copy._present) {
			new(&_data) Type(_copy.get());
		}

		_present = _copy._present;
	}
	optional(optional&& _move)
	{
		if (_move._present) {
			new(&_data) Type(std::move(_move.get()));

			_present = true;
			_move._present = false;
		} else {
			_present = false;
		}
	}
	~optional()
	{
		reset();
	}
	void reset()
	{
		if (_present) {
			_present = false;

			reinterpret_cast<Type*>(&_data)->~Type();
		}
	}
	template<typename Consumer>
	void if_present(Consumer&& _consumer)
	{
		if (_present) {
			_consumer(get());
		}
	}
	template<typename Consumer>
	void if_present(Consumer&& _consumer) const
	{
		if (_present) {
			_consumer(get());
		}
	}
	template<typename... Arguments>
	void emplace(Arguments&&... _arguments)
	{
		reset();

		new(&_data) Type(std::forward<Arguments>(_arguments)...);
	}
	bool is_present() const noexcept
	{
		return _present;
	}
	Type& get()
	{
		if (!_present) {
			throw no_such_element_error();
		}

		return *reinterpret_cast<Type*>(&_data);
	}
	const Type& get() const
	{
		if (!_present) {
			throw no_such_element_error();
		}

		return *reinterpret_cast<const Type*>(&_data);
	}
	Type& or_else(Type& _value)
	{
		if (_present) {
			return get();
		}

		return _value;
	}
	Type or_else(Type&& _value)
	{
		if (_present) {
			return get();
		}

		return _value;
	}
	const Type& or_else(const Type& _value) const
	{
		if (_present) {
			return get();
		}

		return _value;
	}
	template<typename Filter>
	optional filter(Filter&& _filter)
	{
		if (_present && (_filter(get()) == true)) {
			return *this;
		}

		return {};
	}
	template<typename Filter>
	optional filter(Filter&& _filter) const
	{
		if (_present && (_filter(get()) == true)) {
			return *this;
		}

		return {};
	}
	template<typename Mapper>
	optional<typename function_result_t<Mapper, Type>> map(Mapper&& _mapper)
	{
		if (_present) {
			return { _mapper(get()) };
		}

		return {};
	}
	template<typename Mapper>
	optional<typename function_result_t<Mapper, Type>> map(Mapper&& _mapper) const
	{
		if (_present) {
			return { _mapper(get()) };
		}

		return {};
	}
	operator bool() const noexcept
	{
		return is_present();
	}
	Type* operator->()
	{
		return &get();
	}
	const Type* operator->() const
	{
		return &get();
	}

private:
	typename std::aligned_storage<sizeof(Type), alignof(Type)>::type _data;
	bool _present;
};

}

using no_such_element_error = optional_detail::no_such_element_error;

template<typename Type>
using optional = typename optional_detail::optional<Type>;

template<typename Type>
inline optional<Type> make_optional(Type&& _value)
{
	return { std::forward<Type>(_value) };
}