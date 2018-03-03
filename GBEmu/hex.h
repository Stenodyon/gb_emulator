#pragma once

#include <iostream>
#include <type_traits>

namespace // 'unary minus on unsiged will still be unsigned' shenanigans
{
    template <bool B, typename T>
    struct _abs {};

    template <typename T>
    struct _abs<true, T> { // it's unsigned
        static T get_value(T value) { return value; }
    };

    template <typename T>
    struct _abs<false, T> { // it's not unsigned
        static T get_value(T value) {
            if (value < 0)
                return -value;
            return value;
        }
    };

    template <typename T>
    T abs(T value) {
        return _abs<std::is_unsigned<T>::value, T>::get_value(value);
    }
}

template <typename T>
struct hex
{
	T value;

	hex(T value_arg) : value(value_arg) {}
};

template <typename T>
std::ostream & operator<<(std::ostream & out, const hex<T> val)
{
	bool negative = val.value < 0;
	out << (negative ? "-0x" : "0x")
		<< std::uppercase
		<< std::setfill('0')
		<< std::setw(sizeof(T) * 2)
		<< std::hex
		<< +abs(val.value)
		<< std::nouppercase
		<< std::dec;
	return out;
}

template <typename T>
std::string operator+(const std::string & a, const hex<T> b)
{
    std::ostringstream sstream;
    sstream << a << b;
    return sstream.str();
}

template <typename T>
std::string operator+(const char * a, const hex<T> b)
{
    return std::string(a) + b;
}
