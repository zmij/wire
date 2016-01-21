/*
 * fixed_size_io.hpp
 *
 *  Created on: 21 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_DETAIL_FIXED_SIZE_IO_HPP_
#define WIRE_DETAIL_FIXED_SIZE_IO_HPP_

#include <wire/encoding/types.hpp>
#include <iostream>

namespace wire {
namespace encoding {
namespace detail {

template < typename T >
std::ostream&
operator << (std::ostream& os, fixed_size< T > v)
{
	std::ostream::sentry s(os);
	if (s) {
		os << v.value;
	}
	return os;
}

template < typename T >
std::istream&
operator >> (std::istream& is, fixed_size< T >& v)
{
	std::istream::sentry s(is);
	if (s) {
		s >> v.value;
	}
	return is;
}

}  // namespace detail
}  // namespace encoding
}  // namespace wire


#endif /* WIRE_DETAIL_FIXED_SIZE_IO_HPP_ */
