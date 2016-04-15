/*
 * reference_fwd.hpp
 *
 *  Created on: Apr 11, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_REFERENCE_FWD_HPP_
#define WIRE_CORE_REFERENCE_FWD_HPP_

#include <memory>

namespace wire {
namespace core {

class reference;
using reference_ptr = ::std::shared_ptr< reference >;
using reference_weak_ptr = ::std::weak_ptr< reference >;


}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_REFERENCE_FWD_HPP_ */
