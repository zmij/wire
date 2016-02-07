/*
 * object.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: zmij
 */

#include <wire/core/object.hpp>
#include <wire/core/dispatch_request.hpp>
#include <unordered_map>

namespace wire {
namespace core {

namespace {

typedef void(object::*object_dispatch_func)(dispatch_request const&, current const&);
const ::std::unordered_map<::std::string, object_dispatch_func>	object_dispatch_map {
	{ "wire_is_a",	&object::__wire_is_a },
	{ "wire_ping",	&object::__wire_ping },
	{ "wire_type",	&object::__wire_type },
	{ "wire_types",	&object::__wire_types },
};

::std::string OBJECT_TYPE_ID = "::wire::core::object";
::std::vector< ::std::string > OBJECT_TYPE_IDS {
	object::wire_static_type()
};

}  // namespace

bool
object::wire_is_a(std::string const& id, current const&) const
{
	return id == wire_static_type();
}

void
object::wire_ping(current const&) const
{
}

::std::string const&
object::wire_type(current const&) const
{
	return wire_static_type();
}

::std::vector< ::std::string > const &
object::wire_types(current const&) const
{
	return OBJECT_TYPE_IDS;
}

void
object::__wire_is_a(dispatch_request const& req, current const& c)
{
	::std::string arg;
	auto b = req.encaps_start;
	decltype(b) e = req.encaps_end;
	encoding::read(b, e, arg);
	encoding::outgoing out;
	encoding::write(std::back_inserter(out), wire_is_a(arg, c));
	req.result(std::move(out));
}

void
object::__wire_ping(dispatch_request const& req, current const& c)
{
	wire_ping(c);
	encoding::outgoing out;
	req.result(std::move(out));
}

void
object::__wire_type(dispatch_request const& req, current const& c)
{
	encoding::outgoing out;
	encoding::write(std::back_inserter(out), wire_type(c));
	req.result(std::move(out));
}

void
object::__wire_types(dispatch_request const& req, current const& c)
{
	encoding::outgoing out;
	encoding::write(std::back_inserter(out), wire_types(c));
	req.result(std::move(out));
}

void
object::__dispatch(dispatch_request const& req, current const& c)
{
	try {
		if (c.operation.type() == encoding::operation_specs::name_string) {
			auto f = object_dispatch_map.find(c.operation.name());
			if (f != object_dispatch_map.end()) {
				(this->*f->second)(req, c);
			}
		} else {
			throw errors::no_operation(
					wire_static_type(), "::", c.operation.name());
		}
	} catch (...) {
		if (req.exception) {
			req.exception(::std::current_exception());
		}
	}
}

constexpr ::std::string const&
object::wire_static_type()
{
	return OBJECT_TYPE_ID;
}

}  // namespace core
}  // namespace wire
