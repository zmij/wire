/*
 * integral_constants.hpp
 *
 *  Created on: 22 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_UTIL_INTEGRAL_CONSTANTS_HPP_
#define WIRE_UTIL_INTEGRAL_CONSTANTS_HPP_

#include <type_traits>

namespace wire {
namespace util {

template < typename ConstantType, bool Condition, ConstantType ifTrue, ConstantType ifFalse >
struct conditional_constant : ::std::integral_constant<ConstantType, ifTrue> {};

template < typename ConstantType, ConstantType ifTrue, ConstantType ifFalse >
struct conditional_constant< ConstantType, false, ifTrue, ifFalse >
    : ::std::integral_constant<ConstantType, ifFalse> {};

}  /* namespace util */
}  /* namespace wire */

#endif /* WIRE_UTIL_INTEGRAL_CONSTANTS_HPP_ */
