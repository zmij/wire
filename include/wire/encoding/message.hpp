/*
 * message.hpp
 *
 *  Created on: Jan 21, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_MESSAGE_HPP_
#define WIRE_ENCODING_MESSAGE_HPP_

#include <wire/encoding/types.hpp>
#include <wire/encoding/wire_io.hpp>
#include <wire/version.hpp>

namespace wire {
namespace encoding {

struct message {
	enum message_flags {
		request		= 0,
		reply		= 2,
		validate	= 3,
		close		= 4,
		type_bits	= reply | validate | close,
		flag_bits	= ~type_bits,
	};
	static constexpr uint32_t MAGIC_NUMBER =
			('w' << 24) | ('i' << 16) | ('r' << 8) | ('e');

	uint8_t			version_major	= ::wire::PROTOCOL_MAJOR;
	uint8_t			version_minor	= ::wire::PROTOCOL_MINOR;
	uint8_t			encoding_major	= ::wire::ENCODING_MAJOR;
	uint8_t			encoding_minor	= ::wire::ENCODING_MINOR;

	message_flags	flags			= request;
	std::size_t		size			= 0;

	message() = default;
	message(message_flags flags, std::size_t size)
		: flags(flags), size(size)
	{}

	bool
	operator == (message const& rhs) const
	{
		return version_major == rhs.version_major
				&& version_minor == rhs.version_minor
				&& encoding_major == rhs.encoding_major
				&& encoding_minor == rhs.encoding_minor
				&& flags == rhs.flags
				&& size == rhs.size;
	}

	void
	swap(message& rhs)
	{
		using std::swap;
		swap(version_major, rhs.version_major);
		swap(version_minor, rhs.version_minor);
		swap(encoding_major, rhs.encoding_major);
		swap(encoding_minor, rhs.encoding_minor);
		swap(flags, rhs.flags);
		swap(size, rhs.size);
	}

	message_flags
	type() const
	{
		return static_cast<message_flags>(flags & type_bits);
	}
};

}  // namespace encoding
}  // namespace wire

namespace std {
inline void
swap(wire::encoding::message& lhs, wire::encoding::message& rhs)
{
	lhs.swap(rhs);
}

}

namespace wire {
namespace encoding {

template < typename OutputIterator >
void
write(OutputIterator o, message const& v)
{
	write(o, int32_fixed_t(message::MAGIC_NUMBER));
	write(o, PROTOCOL_MAJOR);
	write(o, PROTOCOL_MINOR);
	write(o, v.encoding_major);
	write(o, v.encoding_minor);
	write(o, v.flags);
	write(o, v.size);
}

template < typename InputIterator >
void
read(InputIterator& begin, InputIterator end, message& v)
{
	int32_fixed_t magic;
	read(begin, end, magic);
	if (magic != message::MAGIC_NUMBER) {
		throw errors::invalid_magic_number("Invalid magic number in message header");
	}
	message tmp;
	read(begin, end, tmp.version_major);
	read(begin, end, tmp.version_minor);
	read(begin, end, tmp.encoding_major);
	read(begin, end, tmp.encoding_minor);
	read(begin, end, tmp.flags);
	read(begin, end, tmp.size);
	v.swap(tmp);
}

}  // namespace encoding
}  // namespace wire

#endif /* WIRE_ENCODING_MESSAGE_HPP_ */
