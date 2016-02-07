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
typedef std::shared_ptr< dispatcher_object > dispatcher_ptr;

class object;
typedef std::shared_ptr< object > object_ptr;

}  // namespace core
}  // namespace wire



#endif /* WIRE_CORE_OBJECT_FWD_HPP_ */
