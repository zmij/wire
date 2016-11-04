/*
 * connection_observer_fwd.hpp
 *
 *  Created on: Nov 1, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_CONNECTION_OBSERVER_FWD_HPP_
#define WIRE_CORE_CONNECTION_OBSERVER_FWD_HPP_

#include <memory>
#include <set>

namespace wire {
namespace core {

struct connection_observer;
using connection_observer_ptr = ::std::shared_ptr<connection_observer>;
using connection_observer_set = ::std::set<connection_observer_ptr>;

}  /* namespace core */
}  /* namespace wire */


#endif /* WIRE_CORE_CONNECTION_OBSERVER_FWD_HPP_ */
