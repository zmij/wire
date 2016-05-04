/*
 * callbacks.hpp
 *
 *  Created on: Jan 29, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_CALLBACKS_HPP_
#define WIRE_CORE_CALLBACKS_HPP_

#include <functional>
#include <exception>

namespace wire {
namespace core {
namespace callbacks {

using void_callback         = ::std::function< void() >;

template < typename ... T >
using callback              = ::std::function< void (T ... ) >;

using exception_callback    = callback< ::std::exception_ptr >;


}  // namespace callbacks
}  // namespace core
}  // namespace wire



#endif /* WIRE_CORE_CALLBACKS_HPP_ */
