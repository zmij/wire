/*
 * current.hpp
 *
 *  Created on: Feb 3, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_CURRENT_HPP_
#define WIRE_CORE_CURRENT_HPP_

#include <wire/encoding/message.hpp>
#include <wire/core/context.hpp>
#include <wire/core/endpoint.hpp>

namespace wire {
namespace core {

struct current {
	encoding::operation_specs	operation;
	context_type				context;
	endpoint					peer_endpoint;
};

extern const current no_current;

}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_CURRENT_HPP_ */
