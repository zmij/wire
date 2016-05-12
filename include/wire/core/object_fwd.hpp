/*
 * object_fwd.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_OBJECT_FWD_HPP_
#define WIRE_CORE_OBJECT_FWD_HPP_

#include <memory>

namespace wire {
namespace core {

class dispatcher_object;
using dispatcher_ptr    = ::std::shared_ptr< dispatcher_object >;

class object;
using object_ptr        = ::std::shared_ptr< object >;
using object_weak_ptr   = ::std::weak_ptr< object >;

}  // namespace core
}  // namespace wire



#endif /* WIRE_CORE_OBJECT_FWD_HPP_ */
