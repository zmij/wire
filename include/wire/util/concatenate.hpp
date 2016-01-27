/*
 * stream_feeder.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#ifndef WIRE_UTIL_CONCATENATE_HPP_
#define WIRE_UTIL_CONCATENATE_HPP_

#include <sstream>
#include <string>

namespace wire {
namespace util {
namespace detail {
template < typename T >
void
concatenate( std::ostream& os, T const& arg)
{
	os << arg;
}

template < typename T, typename ... Y >
void
concatenate( std::ostream& os, T const& arg, Y const& ... args )
{
	os << arg;
	concatenate(os, args ...);
}
}  // namespace detail

template < typename ... T >
std::string
concatenate( T const& ... args)
{
	std::ostringstream os;
	detail::concatenate(os, args ...);
	return os.str();
}


}  // namespace util
}  // namespace wire

#endif /* WIRE_UTIL_CONCATENATE_HPP_ */
