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
#include <wire/core/identity.hpp>
#include <wire/core/context.hpp>
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
	write(o,
		int32_fixed_t(message::MAGIC_NUMBER),
		PROTOCOL_MAJOR, PROTOCOL_MINOR,
		v.encoding_major, v.encoding_minor,
		v.flags, v.size);
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
	read(begin, end,
		tmp.version_major, tmp.version_minor,
		tmp.encoding_major, tmp.encoding_minor,
		tmp.flags, tmp.size);
	v.swap(tmp);
}

struct operation_specs {
	typedef boost::variant< int32_t, std::string > operation_id;
	core::identity		identity;
	std::string			facet;
	operation_id		operation;

	void
	swap(operation_specs& rhs)
	{
		using std::swap;
		swap(identity, rhs.identity);
		swap(facet, rhs.facet);
		swap(operation, rhs.operation);
	}
};

template < typename OutputIterator >
void
wire_write(OutputIterator o, operation_specs const& v)
{
	write(o,
		v.identity, v.facet, v.operation
	);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, operation_specs& v)
{
	operation_specs tmp;
	read(begin, end, tmp.identity, tmp.facet, tmp.operation);
	v.swap(tmp);
}


struct request {
	enum request_mode {
		normal
	};
	uint32_t			number;
	operation_specs		operation;
	request_mode		mode;

	void
	swap(request& rhs)
	{
		using std::swap;
		swap(number, rhs.number);
		swap(operation, rhs.operation);
		swap(mode, rhs.mode);
	}
};

template < typename OutputIterator >
void
wire_write(OutputIterator o, request const& v)
{
	write(o, v.number, v.operation, v.mode);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, request& v)
{
	request tmp;
	read(begin, end, tmp.number, tmp.operation, tmp.mode);
	v.swap(tmp);
}

struct reply {
	enum reply_status {
		success,
		user_exception,
		no_object,
		no_facet,
		no_operation,
		unknown_wire_exception,
		unknown_user_exception,
		unknown_exception
	};
	uint32_t		number;
	reply_status	status;

	void
	swap(reply& rhs)
	{
		using std::swap;
		swap(number, rhs.number);
		swap(status, rhs.status);
	}
};

template < typename OutputIterator >
void
wire_write(OutputIterator o, reply const& v)
{
	write(o, v.number, v.status);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, reply& v)
{
	reply tmp;
	read(begin, end, tmp.number, tmp.status);
	v.swap(tmp);
}

}  // namespace encoding
}  // namespace wire

#endif /* WIRE_ENCODING_MESSAGE_HPP_ */
