/*
 * types.hpp
 *
 *  Created on: Dec 11, 2015
 *      Author: zmij
 */

#ifndef TIP_WIRE_TYPES_HPP_
#define TIP_WIRE_TYPES_HPP_

#include <cstdint>
#include <type_traits>

namespace tip {
namespace wire {

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
};

template < typename T >
struct fixed_size : detail::fixed_size_base< T, std::is_integral<T>::value > {
};

}  // namespace detail

typedef detail::fixed_size< int32_t > int32_fixed_t;
typedef detail::fixed_size< uint32_t > uint32_fixed_t;

typedef detail::fixed_size< int64_t > int64_fixed_t;
typedef detail::fixed_size< uint64_t > uint64_fixed_t;

}  // namespace wire
}  // namespace tip



#endif /* TIP_WIRE_TYPES_HPP_ */
