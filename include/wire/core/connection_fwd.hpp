/*
 * connection_fwd.hpp
 *
 *  Created on: Apr 11, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_CONNECTION_FWD_HPP_
#define WIRE_CORE_CONNECTION_FWD_HPP_

#include <memory>
#include <wire/core/invocation_options.hpp>

namespace wire {
namespace core {

class connection;
using connection_ptr        = ::std::shared_ptr< connection >;
using connection_weak_ptr   = ::std::weak_ptr< connection >;

}  // namespace core
}  // namespace wire



#endif /* WIRE_CORE_CONNECTION_FWD_HPP_ */
