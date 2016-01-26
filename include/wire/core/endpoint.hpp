/*
 * endpoint.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_ENDPOINT_HPP_
#define WIRE_CORE_ENDPOINT_HPP_

#include <string>
#include <iosfwd>
#include <boost/variant.hpp>

#include <wire/encoding/wire_io.hpp>

namespace wire {
namespace core {

enum class transport_type {
	tcp,
	ssl,
	udp,
	socket,
};

namespace detail {

struct inet_endpoint_data {
	std::string		host = "";
	uint16_t		port = 0;

	inet_endpoint_data() = default;
	inet_endpoint_data(std::string const& host, uint16_t port)
		: host(host), port(port)
	{
	}

	bool
	operator == (inet_endpoint_data const& rhs) const
	{
		return host == rhs.host && port == rhs.port;
	}
	bool
	operator != (inet_endpoint_data const& rhs) const
	{
		return !(*this == rhs);
	}
};

template < typename OutputIterator >
void
write(OutputIterator o, inet_endpoint_data const& v)
{
	encoding::write(o, v.host);
	encoding::write(o, v.port);
}

template < typename InputIterator >
void
read(InputIterator& begin, InputIterator& end, inet_endpoint_data& v)
{
	encoding::read(begin, end, v.host);
	encoding::read(begin, end, v.port);
}

struct controlled_endpoint_data : inet_endpoint_data {
	uint32_t		timeout = 0;

	controlled_endpoint_data() = default;
	controlled_endpoint_data( std::string const& host, uint16_t port,
			uint32_t timeout = 0)
		: inet_endpoint_data{ host, port }, timeout(timeout)
	{
	}

	bool
	operator == (controlled_endpoint_data const& rhs) const
	{
		return (static_cast< inet_endpoint_data const& >(*this) == static_cast< inet_endpoint_data const& >(rhs))
				&& timeout == rhs.timeout;
	}
	bool
	operator != (controlled_endpoint_data const& rhs) const
	{
		return !(*this == rhs);
	}
};

template < typename OutputIterator >
void
write(OutputIterator o, controlled_endpoint_data const& v)
{
	write(o, static_cast< inet_endpoint_data const& >(v));
	encoding::write(o, v.timeout);
}

template < typename InputIterator >
void
read(InputIterator& begin, InputIterator& end, controlled_endpoint_data& v)
{
	read(begin, end, static_cast< inet_endpoint_data& >(v));
	encoding::read(begin, end, v.timeout);
}

struct tcp_endpoint_data : controlled_endpoint_data {
	tcp_endpoint_data() = default;
	tcp_endpoint_data( std::string const& host, uint16_t port,
			uint32_t timeout = 0)
		: controlled_endpoint_data{host, port, timeout}
	{
	}
};

struct ssl_endpoint_data : controlled_endpoint_data {
	ssl_endpoint_data() = default;
	ssl_endpoint_data( std::string const& host, uint16_t port,
			uint32_t timeout = 0)
		: controlled_endpoint_data{host, port, timeout}
	{
	}
};

struct udp_endpoint_data : inet_endpoint_data {
	udp_endpoint_data() = default;
	udp_endpoint_data( std::string const& host, uint16_t port)
		: inet_endpoint_data{host, port}
	{
	}
};

struct socket_endpoint_data {
	std::string		path;

	bool
	operator == (socket_endpoint_data const& rhs) const
	{
		return path == rhs.path;
	}
	bool
	operator != (socket_endpoint_data const& rhs) const
	{
		return !(*this == rhs);
	}
};

template < typename OutputIterator >
void
write(OutputIterator o, socket_endpoint_data const& v)
{
	encoding::write(o, v.path);
}

template < typename InputIterator >
void
read(InputIterator& begin, InputIterator end, socket_endpoint_data& v)
{
	encoding::read(begin, end, v.path);
}

}  // namespace detail

class endpoint {
public:
	typedef boost::variant<
		detail::tcp_endpoint_data,
		detail::ssl_endpoint_data,
		detail::udp_endpoint_data,
		detail::socket_endpoint_data
	> endpoint_data;
public:
	template < typename T >
	endpoint( T&& data ) : endpoint_data_(std::forward<T&&>(data)) {}

	transport_type
	transport() const
	{
		return static_cast<transport_type>(endpoint_data_.which());
	}

	template < typename OutputIterator >
	void
	write( OutputIterator o ) const
	{
		encoding::write(o, endpoint_data_);
	}
	template < typename InputIterator >
	void
	read( InputIterator& begin, InputIterator end)
	{
		encoding::read(begin, end, endpoint_data_);
	}
private:
	endpoint_data	endpoint_data_;
};

template < typename OutputIterator >
void
write(OutputIterator o, endpoint const& v)
{
	v.write(o);
}

template < typename InputIterator >
void
read(InputIterator& begin, InputIterator end, endpoint& v)
{
	v.read(begin, end);
}

//----------------------------------------------------------------------------
std::ostream&
operator << (std::ostream& os, transport_type val);
std::istream&
operator >> (std::istream& in, transport_type& val);

}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_ENDPOINT_HPP_ */
