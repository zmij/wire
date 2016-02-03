/*
 * buffers_fwd.hpp
 *
 *  Created on: Feb 3, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_BUFFERS_FWD_HPP_
#define WIRE_ENCODING_BUFFERS_FWD_HPP_

#include <memory>

namespace wire {
namespace encoding {

class outgoing;
typedef std::shared_ptr< outgoing > outgoing_ptr;

class incoming;
typedef std::shared_ptr< incoming > incoming_ptr;

}  // namespace encoding
}  // namespace wire

#endif /* WIRE_ENCODING_BUFFERS_FWD_HPP_ */
