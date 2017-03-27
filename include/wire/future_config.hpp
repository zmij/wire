/*
 * future_config.hpp
 *
 *  Created on: Mar 25, 2017
 *      Author: zmij
 */

#ifndef WIRE_FUTURE_CONFIG_HPP_
#define WIRE_FUTURE_CONFIG_HPP_

#ifdef WITH_BOOST_FIBERS
#include <boost/fiber/future.hpp>
#include <boost/fiber/fiber.hpp>
#else
#include <future>
#endif

namespace wire {

#ifdef WITH_BOOST_FIBERS
template < typename _Res >
using promise = ::boost::fibers::promise< _Res >;
using fiber = ::boost::fibers::fiber;
#else
template < typename _Res >
using promise = ::std::promise< _Res >;
#endif

} /* namespace wire */


#endif /* WIRE_FUTURE_CONFIG_HPP_ */
