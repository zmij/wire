/*
 * proxy_fwd.hpp
 *
 *  Created on: Jan 29, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_PROXY_FWD_HPP_
#define WIRE_CORE_PROXY_FWD_HPP_

#include <memory>

namespace wire {
namespace core {

class object_proxy;
typedef std::shared_ptr<object_proxy> object_prx;

}  // namespace core
}  // namespace wire


#endif /* WIRE_CORE_PROXY_FWD_HPP_ */
