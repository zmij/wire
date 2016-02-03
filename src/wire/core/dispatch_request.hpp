/*
 * dispatch_request.hpp
 *
 *  Created on: Feb 3, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_DISPATCH_REQUEST_HPP_
#define WIRE_CORE_DISPATCH_REQUEST_HPP_

#include <wire/core/dispatch_request_fwd.hpp>
#include <wire/core/callbacks.hpp>
#include <wire/encoding/buffers.hpp>

namespace wire {
namespace core {

struct dispatch_request {
	encoding::incoming_ptr 				buffer;
	encoding::incoming::const_iterator	encaps_start;
	encoding::incoming::size_type		encaps_size;

	encoding::request_result_callback	result;
	callbacks::exception_callback		exception;
};

}  // namespace core
}  // namespace wire



#endif /* WIRE_CORE_DISPATCH_REQUEST_HPP_ */
