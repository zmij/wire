/*
 * adapter_fwd.hpp
 *
 *  Created on: 7 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_CORE_ADAPTER_FWD_HPP_
#define WIRE_CORE_ADAPTER_FWD_HPP_

#include <memory>

namespace wire {
namespace core {

class adapter;
typedef ::std::shared_ptr<adapter> adapter_ptr;
typedef ::std::weak_ptr<adapter> adapter_weak_ptr;

}  // namespace core
}  // namespace wire


#endif /* WIRE_CORE_ADAPTER_FWD_HPP_ */
