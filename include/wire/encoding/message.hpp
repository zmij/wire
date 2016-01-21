/*
 * message.hpp
 *
 *  Created on: Jan 21, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_MESSAGE_HPP_
#define WIRE_ENCODING_MESSAGE_HPP_

#include <wire/encoding/types.hpp>
#include <wire/encoding/buffers.hpp>
#include <wire/version.hpp>

namespace wire {
namespace encoding {

struct message {
	static constexpr uint32_fixed_t	magic_numer =
			('w' << 24) | ('i' << 16) | ('r' << 8) | ('e');
	static constexpr uint8_t version_major = VERSION_MAJOR;
	static constexpr uint8_t version_minor = VERSION_MINOR;
};

template < typename OutputIterator >
void
write(OutputIterator o, message const& v)
{
	detail::write(o, message::magic_numer);
	detail::write(o, message::version_major);
	detail::write(o, message::version_minor);
}

}  // namespace encoding
}  // namespace wire

#endif /* WIRE_ENCODING_MESSAGE_HPP_ */
