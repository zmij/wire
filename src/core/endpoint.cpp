/*
 * endpoint.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: zmij
 */

#include <wire/core/endpoint.hpp>
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
	const std::map< transport_type, std::string > CLASS_TO_STRING {
		{ transport_type::tcp, "tcp" },
		{ transport_type::ssl, "ssl" },
		{ transport_type::udp, "udp" },
		{ transport_type::socket, "socket" },
	}; // CLASS_TO_STRING
	const std::map< std::string, transport_type > STRING_TO_CLASS {
		{ "tcp", transport_type::tcp },
		{ "ssl", transport_type::ssl },
		{ "udp", transport_type::udp },
		{ "socket", transport_type::socket },
	}; // STRING_TO_CLASS
} // namespace

// Generated output operator
std::ostream&
operator << (std::ostream& out, transport_type val)
{
	std::ostream::sentry s (out);
	if (s) {
		auto f = CLASS_TO_STRING.find(val);
		if (f != CLASS_TO_STRING.end()) {
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
			auto f = STRING_TO_CLASS.find(name);
			if (f != STRING_TO_CLASS.end()) {
				val = f->second;
			} else {
				in.setstate(std::ios_base::failbit);
			}
		}
	}
	return in;
}

}  // namespace core
}  // namespace wire
