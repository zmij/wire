/*
 * message.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: zmij
 */

#include <wire/encoding/message.hpp>

namespace wire {
namespace encoding {

namespace {
	const std::map< message::message_flags, std::string > MESSAGE_FLAGS_TO_STRING {
		{ message::request, "request" },
		{ message::reply, "reply" },
		{ message::validate, "validate" },
		{ message::close, "close" },
	}; // MESSAGE_FLAGS_TO_STRING
	const std::map< std::string, message::message_flags > STRING_TO_MESSAGE_FLAGS {
		{ "request", message::request },
		{ "reply", message::reply },
		{ "validate", message::validate },
		{ "close", message::close },
	}; // STRING_TO_MESSAGE_FLAGS
} // namespace

std::ostream&
operator << (std::ostream& out, message::message_flags val)
{
	std::ostream::sentry s (out);
	if (s) {
		auto f = MESSAGE_FLAGS_TO_STRING.find(val);
		if (f != MESSAGE_FLAGS_TO_STRING.end()) {
			out << f->second;
		} else {
			out << "Unknown message_flags " << (int)val;
		}
	}
	return out;
}

}  // namespace encoding
}  // namespace wire
