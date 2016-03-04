/*
 * endpoint.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: zmij
 */

#include <wire/core/endpoint.hpp>
#include <wire/core/grammar/endpoint_parse.hpp>

#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>

#include <iostream>

namespace wire {
namespace core {
namespace detail {
static_assert(
	encoding::detail::wire_type< tcp_endpoint_data >::value == encoding::detail::STRUCT,
	"Wire type for tcp endpoint data is STRUCT");

static_assert(
	encoding::detail::wire_type< ssl_endpoint_data >::value == encoding::detail::STRUCT,
	"Wire type for ssl endpoint data is STRUCT");

static_assert(
	encoding::detail::wire_type< udp_endpoint_data >::value == encoding::detail::STRUCT,
	"Wire type for udp endpoint data is STRUCT");

static_assert(
	encoding::detail::wire_type< socket_endpoint_data >::value == encoding::detail::STRUCT,
	"Wire type for socket endpoint data is STRUCT");
}  // namespace detail

static_assert(
	encoding::detail::wire_type< endpoint >::value == encoding::detail::STRUCT,
	"Wire type for endpoint data is STRUCT");


namespace {
	const std::map< transport_type, std::string > TRANSPORT_TYPE_TO_STRING {
		{ transport_type::empty, "empty" },
		{ transport_type::tcp, "tcp" },
		{ transport_type::ssl, "ssl" },
		{ transport_type::udp, "udp" },
		{ transport_type::socket, "socket" },
	}; // TRANSPORT_TYPE_TO_STRING
	const std::map< std::string, transport_type > STRING_TO_TRANSPORT_TYPE {
		{ "empty", transport_type::empty },
		{ "tcp", transport_type::tcp },
		{ "ssl", transport_type::ssl },
		{ "udp", transport_type::udp },
		{ "socket", transport_type::socket },
	}; // STRING_TO_TRANSPORT_TYPE
} // namespace

// Generated output operator
std::ostream&
operator << (std::ostream& out, transport_type val)
{
	std::ostream::sentry s (out);
	if (s) {
		auto f = TRANSPORT_TYPE_TO_STRING.find(val);
		if (f != TRANSPORT_TYPE_TO_STRING.end()) {
			out << f->second;
		} else {
			out << "Unknown class " << (int)val;
		}
	}
	return out;
}
// Generated input operator
std::istream&
operator >> (std::istream& in, transport_type& val)
{
	std::istream::sentry s (in);
	if (s) {
		std::string name;
		if (in >> name) {
			auto f = STRING_TO_TRANSPORT_TYPE.find(name);
			if (f != STRING_TO_TRANSPORT_TYPE.end()) {
				val = f->second;
			} else {
				in.setstate(std::ios_base::failbit);
			}
		}
	}
	return in;
}

namespace detail {

void
inet_endpoint_data::check() const
{
	if (host.empty()) {
		throw errors::logic_error("Empty host in an endpoint");
	}
	if (port == 0) {
		throw errors::logic_error("Port is not set in an endpoint");
	}
}

void
inet_endpoint_data::check(transport_type t) const
{
	if (host.empty()) {
		throw errors::logic_error("Empty host in ", t, " endpoint");
	}
	if (port == 0) {
		throw errors::logic_error("Port is not set in ", t ," endpoint");
	}
}

void
socket_endpoint_data::check(transport_type t) const
{
	if (path.empty()) {
		throw errors::logic_error("Empty socket path in ", t, " endpoint");
	}
}

std::ostream&
operator << (std::ostream& os, empty_endpoint const& val)
{
	return os;
}


std::ostream&
operator << (std::ostream& os, inet_endpoint_data const& val)
{
	std::ostream::sentry s (os);
	if (s) {
		os << val.host << ':' << val.port;
	}
	return os;
}

std::ostream&
operator << (std::ostream& os, socket_endpoint_data const& val)
{
	std::ostream::sentry s (os);
	if (s) {
		os << val.path;
	}
	return os;
}

::std::size_t
hash_value(inet_endpoint_data const& val)
{
	return ::std::hash<::std::string>()(val.host) ^
			(::std::hash<uint16_t>()(val.port) << 1);
}

::std::size_t
hash_value(socket_endpoint_data const& val)
{
	return ::std::hash<::std::string>()(val.path);
}

}  // namespace detail

void
endpoint::swap(endpoint& rhs)
{
	using std::swap;
	swap(endpoint_data_, rhs.endpoint_data_);
}

endpoint&
endpoint::operator =(endpoint const& rhs)
{
	endpoint(rhs).swap(*this);
	return *this;
}

endpoint&
endpoint::operator =(endpoint&& rhs)
{
	endpoint(std::move(rhs)).swap(*this);
	return *this;
}

namespace detail {
struct endpoint_data_check : boost::static_visitor<> {
	transport_type expected_type;

	endpoint_data_check( transport_type e) : expected_type{e} {}
	void
	operator()( empty_endpoint const& data ) const
	{
		throw errors::logic_error( "Empty endpoint is not connectible" );
	}
	void
	operator()( inet_endpoint_data const& data ) const
	{
		data.check(expected_type);
	}
	void
	operator()( socket_endpoint_data const& data ) const
	{
		data.check(expected_type);
	}
};
}  // namespace detail

void
endpoint::check(transport_type expected) const
{
	if (transport() != expected) {
		throw errors::logic_error("Invalid endpoint transport type ",
				transport(),
				" for ", expected, " transport");
	}
	boost::apply_visitor(detail::endpoint_data_check{ expected }, endpoint_data_);
}

std::ostream&
operator << (std::ostream& os, endpoint const& val)
{
	std::ostream::sentry s(os);
	if (s) {
		os << val.transport() << "://" << val.data();
	}
	return os;
}

std::istream&
operator >> (std::istream& is, endpoint& val)
{
	typedef std::istreambuf_iterator<char> istreambuf_iterator;
	typedef boost::spirit::multi_pass< istreambuf_iterator > stream_iterator;
	typedef grammar::parse::endpoint_grammar< stream_iterator > endpoint_grammar;
	static endpoint_grammar endpoint_parser;

	std::istream::sentry s(is);
	if (s) {
		stream_iterator in = stream_iterator(istreambuf_iterator(is));
		stream_iterator eos = stream_iterator(istreambuf_iterator());
		endpoint tmp;
		if (boost::spirit::qi::parse(in, eos, endpoint_parser, tmp)) {
			tmp.swap(val);
		} else {
			is.setstate(std::ios_base::failbit);
		}
	}
	return is;
}

::std::istream&
operator >> (::std::istream& is, endpoint_list& val)
{
	typedef std::istreambuf_iterator<char> istreambuf_iterator;
	typedef boost::spirit::multi_pass< istreambuf_iterator > stream_iterator;
	typedef grammar::parse::endpoints_grammar< stream_iterator > endpoints_grammar;
	static endpoints_grammar endpoints_parser;

	std::istream::sentry s(is);
	if (s) {
		stream_iterator in = stream_iterator(istreambuf_iterator(is));
		stream_iterator eos = stream_iterator(istreambuf_iterator());
		endpoint_list tmp;
		if (boost::spirit::qi::parse(in, eos, endpoints_parser, tmp)) {
			tmp.swap(val);
		} else {
			is.setstate(::std::ios_base::failbit);
		}
	}
	return is;
}

endpoint
endpoint::tcp(std::string const& host, uint16_t port)
{
	return std::move(endpoint{ detail::tcp_endpoint_data{ host, port } });
}

endpoint
endpoint::ssl(std::string const& host, uint16_t port)
{
	return std::move(endpoint{ detail::ssl_endpoint_data{ host, port } });
}
endpoint
endpoint::udp(std::string const& host, uint16_t port)
{
	return std::move(endpoint{ detail::udp_endpoint_data{ host, port } });
}
endpoint
endpoint::socket(std::string const& path)
{
	return std::move(endpoint{ detail::socket_endpoint_data{ path } });
}

namespace detail {
struct endpoint_hash_visitor : boost::static_visitor<::std::size_t> {
	template < typename E >
	::std::size_t
	operator()( E const& data ) const
	{
		return hash_value(data);
	}
};
}  // namespace detail

::std::size_t
hash_value(endpoint const& val)
{
	return ::boost::apply_visitor(detail::endpoint_hash_visitor{}, val.data());
}

}  // namespace core
}  // namespace wire
