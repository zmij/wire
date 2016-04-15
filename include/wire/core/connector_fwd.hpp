/*
 * connector_fwd.hpp
 *
 *  Created on: 7 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_CORE_CONNECTOR_FWD_HPP_
#define WIRE_CORE_CONNECTOR_FWD_HPP_

#include <memory>

namespace wire {
namespace core {

class connector;
using connector_ptr         = ::std::shared_ptr< connector >;
using connector_weak_ptr    = ::std::weak_ptr< connector >;

}  // namespace core
}  // namespace wire


#endif /* WIRE_CORE_CONNECTOR_FWD_HPP_ */
