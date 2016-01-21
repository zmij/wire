/*
 * wire.hpp
 *
 *  Created on: Dec 11, 2015
 *      Author: zmij
 */

#ifndef TIP_WIRE_DETAIL_SIZE_HPP_
#define TIP_WIRE_DETAIL_SIZE_HPP_

#include <wire/encoding/types.hpp>
#include <wire/encoding/detail/bits.hpp>

namespace tip {
namespace wire {

template < typename T >
uint32_t
wire_size(T const&);

namespace detail {

template < typename T, wire_types >
struct wire_size_impl;

template < typename T >
struct wire_size_impl < T, SCALAR_FIXED > {
	typedef T type;
	static uint32_t
	size(type v)
	{
		return sizeof(T);
	}
};

template < typename T >
struct wire_size_impl < T, SCALAR_VARINT > {
	typedef T type;
	static uint32_t
	size(type v)
	{
		return bits::significant_bits(v) / 7 + 1;
	}
};

template <>
struct wire_size_impl < std::string, SCALAR_LENGTH_DELIM > {
	static uint32_t
	size(std::string const& v)
	{
		std::string::size_type sz = v.size();
		return wire::wire_size(sz) + sz;
	}
};

template < typename T >
struct wire_size : wire_size_impl< T, wire_type<T>::value > {};

template < typename T >
struct wire_size< wire::detail::fixed_size< T > > {
	typedef wire::detail::fixed_size< T > type;

	static int32_t
	size(type const& v)
	{
		return type::size;
	}
};

}  // namespace detail

template < typename T >
uint32_t
wire_size(T const& v)
{
	return detail::wire_size<T>::size(v);
}

}  // namespace wire
}  // namespace tip


#endif /* TIP_WIRE_DETAIL_SIZE_HPP_ */
