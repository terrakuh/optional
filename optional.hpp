/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/
#pragma once

#include <type_traits>
#include <utility>
#include <stdexcept>
#include <functional>


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
	template<typename Consumer, typename Runnable>
	void if_present_or_else(Consumer&& _consumer, Runnable&& _runnable)
	{
		if (_present) {
			_consumer(get());
		} else {
			_runnable();
		}
	}
	template<typename Consumer, typename Runnable>
	void if_present_or_else(Consumer&& _consumer, Runnable&& _runnable) const
	{
		if (_present) {
			_consumer(get());
		} else {
			_runnable();
		}
	}
	template<typename... Arguments>
	void emplace(Arguments&&... _arguments)
	{
		reset();

		new(&_data) Type(std::forward<Arguments>(_arguments)...);
	}
	bool present() const noexcept
	{
		return _present;
	}
	bool empty() const noexcept
	{
		return !_present;
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
	template<typename Supplier>
	typename std::conditional<
		std::is_reference<typename function_result_t<Supplier>>::value,
		Type&,
		Type>::type
		or_else_get(Supplier&& _supplier)
	{
		if (_present) {
			return get();
		}

		return _supplier();
	}
	template<typename Supplier>
	typename std::conditional<
		std::is_reference<typename function_result_t<Supplier>>::value,
		const Type&,
		Type>::type
		or_else_get(Supplier&& _supplier) const
	{
		if (_present) {
			return get();
		}

		return _supplier();
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
		return present();
	}
	operator Type&()
	{
		return get();
	}
	operator const Type&() const
	{
		return get();
	}
	Type* operator->()
	{
		return &get();
	}
	const Type* operator->() const
	{
		return &get();
	}
	optional& operator=(const optional& _copy)
	{
		reset();

		if (_copy._present) {
			new(&_data) Type(_copy.get());

			_present = true;
		}

		return *this;
	}
	optional& operator=(optional&& _move)
	{
		reset();

		if (_move._present) {
			new(&_data) Type(std::move(_move.get()));

			_present = true;
			_move._present = false;
		}

		return *this;
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

namespace std
{

template<typename Type>
struct hash<optional<Type>>
{
	std::size_t operator()(const optional<Type>& _value) const
	{
		return _value ? std::hash<Type>()(_value.get()) : 0;
	}
};

}
