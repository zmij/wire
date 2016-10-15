/*
 * object.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: zmij
 */

#include <wire/core/object.hpp>
#include <wire/core/detail/dispatch_request.hpp>
#include <wire/errors/not_found.hpp>
#include <unordered_map>

namespace wire {
namespace core {

namespace {

::std::string const WIRE_CORE_OBJECT_wire_is_a = "wire_is_a";
::std::string const WIRE_CORE_OBJECT_wire_ping = "wire_ping";
::std::string const WIRE_CORE_OBJECT_wire_type = "wire_type";
::std::string const WIRE_CORE_OBJECT_wire_types = "wire_types";

using object_dispatch_func = void(object::*)(detail::dispatch_request const&, current const&);
::std::unordered_map<::std::string, object_dispatch_func> const object_dispatch_map {
    { WIRE_CORE_OBJECT_wire_is_a,    &object::__wire_is_a },
    { WIRE_CORE_OBJECT_wire_ping,    &object::__wire_ping },
    { WIRE_CORE_OBJECT_wire_type,    &object::__wire_type },
    { WIRE_CORE_OBJECT_wire_types,    &object::__wire_types },
};

::std::string OBJECT_TYPE_ID = "::wire::core::object";
::std::vector< ::std::string > OBJECT_TYPE_IDS {
    object::wire_static_type_id()
};

}  // namespace

bool
object::wire_is_a(std::string const& id, current const&) const
{
    return id == wire_static_type_id();
}

void
object::wire_ping(current const&) const
{
}

::std::string const&
object::wire_type(current const&) const
{
    return wire_static_type_id();
}

::std::vector< ::std::string > const &
object::wire_types(current const&) const
{
    return OBJECT_TYPE_IDS;
}

void
object::__wire_is_a(detail::dispatch_request const& req, current const& c)
{
    ::std::string arg;
    auto b = req.encaps_start;
    decltype(b) e = req.encaps_end;
    encoding::read(b, e, arg);
    req.encaps_start.incoming_encapsulation().read_indirection_table(b);
    encoding::outgoing out{ req.buffer->get_connector() };
    encoding::write(std::back_inserter(out), wire_is_a(arg, c));
    req.result(std::move(out));
}

void
object::__wire_ping(detail::dispatch_request const& req, current const& c)
{
    wire_ping(c);
    encoding::outgoing out{ req.buffer->get_connector() };
    req.result(std::move(out));
}

void
object::__wire_type(detail::dispatch_request const& req, current const& c)
{
    encoding::outgoing out{ req.buffer->get_connector() };
    encoding::write(std::back_inserter(out), wire_type(c));
    req.result(std::move(out));
}

void
object::__wire_types(detail::dispatch_request const& req, current const& c)
{
    encoding::outgoing out{ req.buffer->get_connector() };
    encoding::write(std::back_inserter(out), wire_types(c));
    req.result(std::move(out));
}

bool
object::__wire_dispatch(detail::dispatch_request const& req, current const& c,
        dispatch_seen_list& seen, bool throw_not_found)
{
    if (seen.count(wire_static_type_id_hash()))
        return false;
    seen.insert(wire_static_type_id_hash());
    if (c.operation.type() == encoding::operation_specs::name_string) {
        auto f = object_dispatch_map.find(c.operation.name());
        if (f != object_dispatch_map.end()) {
            (this->*f->second)(req, c);
            return true;
        }
        if (throw_not_found)
            throw errors::no_operation(
                    c.operation.target.identity, c.operation.target.facet, c.operation.name());
        return false;
    } else {
        throw errors::no_operation(
                c.operation.target.identity, c.operation.target.facet, c.operation.name());
    }
}

void
object::__dispatch(detail::dispatch_request const& req, current const& c)
{
    try {
        dispatch_seen_list seen;
        if (!__wire_dispatch(req, c, seen, true)) {
            throw errors::no_operation{
                wire_static_type_id(), "::", c.operation.name()};
        }
    } catch (...) {
        if (req.exception) {
            req.exception(::std::current_exception());
        }
    }
}

::std::string const&
object::wire_static_type_id()
{
    return OBJECT_TYPE_ID;
}

::std::int64_t
object::wire_static_type_id_hash()
{
    return 0;
}

}  // namespace core
}  // namespace wire
