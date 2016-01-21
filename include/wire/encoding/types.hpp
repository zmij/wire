/*
 * types.hpp
 *
 *  Created on: Dec 11, 2015
 *      Author: zmij
 */

#ifndef WIRE_TYPES_HPP_
#define WIRE_TYPES_HPP_

#include <cstdint>
#include <type_traits>

namespace wire {
namespace encoding {
namespace detail {

template < typename T, bool is_integral >
struct fixed_size_base;

template < typename T >
struct fixed_size_base < T, true > {
	typedef T type;
	enum {
		size = sizeof(T)
	};
	type value;

	operator type() const { return value; }
};

template < typename T >
struct fixed_size
	: fixed_size_base< T, std::is_integral<T>::value > {
	typedef fixed_size_base< T, std::is_integral<T>::value >	base_type;
	typedef typename base_type::type							type;
	fixed_size() : base_type{0} {}
	fixed_size(type v) : base_type{v} {}
	fixed_size(fixed_size const&) = default;

	fixed_size&
	operator = (fixed_size const&) = default;
	fixed_size&
	operator = (type const& v)
	{
		base_type::value = v;
		return *this;
	}
};



}  // namespace detail

typedef detail::fixed_size< int32_t > int32_fixed_t;
typedef detail::fixed_size< uint32_t > uint32_fixed_t;

typedef detail::fixed_size< int64_t > int64_fixed_t;
typedef detail::fixed_size< uint64_t > uint64_fixed_t;

}  // namespace encoding
}  // namespace wire


#endif /* WIRE_TYPES_HPP_ */
