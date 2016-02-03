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

struct version {
	uint32_t	major;
	uint32_t	minor;

	bool
	operator == (version const& rhs) const
	{
		return major == rhs.major && minor == rhs.minor;
	}

	bool
	operator != (version const& rhs) const
	{
		return !(*this == rhs);
	}

	bool
	operator < (version const& rhs) const
	{
		return major < rhs.major || (major == rhs.major && minor < rhs.minor);
	}
	bool
	operator <= (version const& rhs) const
	{
		return major <= rhs.major || (major == rhs.major && minor <= rhs.minor);
	}

	bool
	operator > (version const& rhs) const
	{
		return rhs < *this;
	}
	bool
	operator >= (version const& rhs) const
	{
		return rhs <= *this;
	}


	void
	swap(version& rhs)
	{
		using std::swap;
		swap(major, rhs.major);
		swap(minor, rhs.minor);
	}
};

template < typename OutputIterator >
void
wire_write(OutputIterator o, version const& v)
{
	write(o, v.major, v.minor);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, version& v)
{
	version tmp;
	read(begin, end, tmp.major, tmp.minor);
	v.swap(tmp);
}


struct message {
	typedef uint64_t	size_type;
	enum message_flags {
		// Types
		request				= 0,
		reply				= 2,
		validate			= 3,
		close				= 4,
		// Flags
		protocol			= 8,		/**< Message header contains protocol version */
		encoding			= 0x10,		/**< Message header contains encoding version */

		// Combinations
		validate_flags		= validate | protocol | encoding,
		// Masks
		type_bits			= reply | validate | close,
		flag_bits			= ~type_bits,
	};
	static constexpr uint32_t MAGIC_NUMBER =
			('w') | ('i' << 8) | ('r' << 16) | ('e' << 24);

	version			protocol_version = version{ ::wire::PROTOCOL_MAJOR, ::wire::PROTOCOL_MINOR };
	version			encoding_version = version{ ::wire::ENCODING_MAJOR, ::wire::ENCODING_MINOR };

	message_flags	flags			= request;
	size_type		size			= 0;

	message() = default;
	message(message_flags flags, size_type size)
		: flags(flags), size(size)
	{}

	bool
	operator == (message const& rhs) const
	{
		return protocol_version == rhs.protocol_version
				&& encoding_version == rhs.encoding_version
				&& flags == rhs.flags
				&& size == rhs.size;
	}

	void
	swap(message& rhs)
	{
		using std::swap;
		swap(protocol_version, rhs.protocol_version);
		swap(encoding_version, rhs.encoding_version);
		swap(flags, rhs.flags);
		swap(size, rhs.size);
	}

	message_flags
	type() const
	{
		return static_cast<message_flags>(flags & type_bits);
	}
};

std::ostream&
operator << (std::ostream& out, message::message_flags val);

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
wire_write(OutputIterator o, message const& v)
{
	write(o, int32_fixed_t(message::MAGIC_NUMBER));
	write(o, v.flags);
	// write this info depending on flags
	if (v.flags & message::protocol)
		write(o, PROTOCOL_MAJOR, PROTOCOL_MINOR);
	if (v.flags & message::encoding)
		write(o, v.encoding_version);

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
	read(begin, end, tmp.flags);
	// read this info depending on message flags
	if (tmp.flags & message::protocol)
		read(begin, end, tmp.protocol_version);
	if (tmp.flags & message::encoding)
		read(begin, end, tmp.encoding_version);

	read(begin, end, tmp.size);
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

	bool
	operator == (operation_specs const& rhs) const
	{
		return identity == rhs.identity && facet == rhs.facet && operation == rhs.operation;
	}

	bool
	operator != (operation_specs const& rhs) const
	{
		return !(*this == rhs);
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

	bool
	operator == (request const& rhs) const
	{
		return number == rhs.number && operation == rhs.operation && mode == rhs.mode;
	}

	bool
	operator != (request const& rhs) const
	{
		return !(*this == rhs);
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


	bool
	operator == (reply const& rhs) const
	{
		return number == rhs.number && status == rhs.status;
	}

	bool
	operator != (reply const& rhs) const
	{
		return !(*this == rhs);
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
