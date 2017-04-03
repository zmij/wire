/*
 * future_traits.hpp
 *
 *  Created on: Apr 3, 2017
 *      Author: zmij
 */

#ifndef WIRE_CORE_DETAIL_FUTURE_TRAITS_HPP_
#define WIRE_CORE_DETAIL_FUTURE_TRAITS_HPP_

#include <future>
#include <type_traits>

namespace wire {
namespace core {
namespace detail {

template < template<typename> class _Promise >
struct promise_want_io_throttle : ::std::false_type {};

template <>
struct promise_want_io_throttle<::std::promise> : ::std::true_type {};

} /* namespace detail */
} /* namespace core */
} /* namespace wire */


#endif /* WIRE_CORE_DETAIL_FUTURE_TRAITS_HPP_ */
