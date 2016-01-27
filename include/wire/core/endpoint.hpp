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
#include <wire/encoding/detail/variant_io.hpp>

namespace wire {
namespace core {

enum class transport_type {
	empty,
	tcp,
	ssl,
	udp,
	socket,
};

namespace detail {

struct empty_endpoint {
	bool
	operator == (empty_endpoint const&) const
	{ return true; }
	bool
	operator != (empty_endpoint const&) const
	{ return false; }
};

std::ostream&
operator << (std::ostream& os, empty_endpoint const& val);

template < typename OutputIterator >
void
wire_write(OutputIterator o, empty_endpoint const& v)
{
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator read, empty_endpoint& v)
{
}

struct inet_endpoint_data {
	std::string		host = "";
	uint16_t		port = 0;

	inet_endpoint_data()
		: host(), port(0)
	{
	}
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

std::ostream&
operator << (std::ostream& os, inet_endpoint_data const& val);

template < typename OutputIterator >
void
wire_write(OutputIterator o, inet_endpoint_data const& v)
{
	encoding::write(o, v.host);
	encoding::write(o, v.port);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator& end, inet_endpoint_data& v)
{
	encoding::read(begin, end, v.host);
	encoding::read(begin, end, v.port);
}

struct controlled_endpoint_data : inet_endpoint_data {
	uint32_t		timeout = 0;

	controlled_endpoint_data() : inet_endpoint_data{}, timeout(0) {}
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

std::ostream&
operator << (std::ostream& os, controlled_endpoint_data const& val);

template < typename OutputIterator >
void
wire_write(OutputIterator o, controlled_endpoint_data const& v)
{
	wire_write(o, static_cast< inet_endpoint_data const& >(v));
	encoding::write(o, v.timeout);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator& end, controlled_endpoint_data& v)
{
	wire_read(begin, end, static_cast< inet_endpoint_data& >(v));
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

std::ostream&
operator << (std::ostream& os, socket_endpoint_data const& val);

template < typename OutputIterator >
void
wire_write(OutputIterator o, socket_endpoint_data const& v)
{
	encoding::write(o, v.path);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, socket_endpoint_data& v)
{
	encoding::read(begin, end, v.path);
}

}  // namespace detail

class endpoint {
public:
	typedef boost::variant<
		detail::empty_endpoint,
		detail::tcp_endpoint_data,
		detail::ssl_endpoint_data,
		detail::udp_endpoint_data,
		detail::socket_endpoint_data
	> endpoint_data;
public:
	endpoint() : endpoint_data_{ detail::empty_endpoint{} } {}
	endpoint(endpoint const& rhs) : endpoint_data_{ rhs.endpoint_data_ } {}
	endpoint(endpoint&& rhs) : endpoint_data_{ std::move(rhs.endpoint_data_) } {}
	template < typename T >
	explicit endpoint( T&& data ) : endpoint_data_{ std::forward<T&&>(data) } {}

	void
	swap(endpoint& rhs);

	endpoint&
	operator = ( endpoint const& );
	endpoint&
	operator = ( endpoint&& );

	bool
	operator == (endpoint const& rhs) const
	{
		return endpoint_data_ == rhs.endpoint_data_;
	}
	bool
	operator != (endpoint const& rhs) const
	{
		return !(*this == rhs);
	}

	transport_type
	transport() const
	{
		return static_cast<transport_type>(endpoint_data_.which());
	}

	endpoint_data&
	data()
	{ return endpoint_data_; }
	endpoint_data const&
	data() const
	{ return endpoint_data_; }

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
wire_write(OutputIterator o, endpoint const& v)
{
	v.write(o);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, endpoint& v)
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
