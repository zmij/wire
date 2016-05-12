/*
 * object_locator_fwd.hpp
 *
 *  Created on: 13 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_CORE_OBJECT_LOCATOR_FWD_HPP_
#define WIRE_CORE_OBJECT_LOCATOR_FWD_HPP_

#include <memory>

namespace wire {
namespace core {

class object_locator;
using object_locator_ptr = ::std::shared_ptr< object_locator >;

}  /* namespace core */
}  /* namespace wire */

#endif /* WIRE_CORE_OBJECT_LOCATOR_FWD_HPP_ */
