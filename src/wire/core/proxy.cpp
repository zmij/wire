/*
 * proxy.cpp
 *
 *  Created on: Jan 29, 2016
 *      Author: zmij
 */

#include <wire/core/proxy.hpp>
#include <wire/core/reference.hpp>
#include <wire/core/connection.hpp>
#include <wire/core/invocation.hpp>
#include <wire/core/object.hpp>

namespace wire {
namespace core {

namespace {

::std::string const WIRE_CORE_OBJECT_wire_is_a = "wire_is_a";
::std::string const WIRE_CORE_OBJECT_wire_ping = "wire_ping";
::std::string const WIRE_CORE_OBJECT_wire_type = "wire_type";
::std::string const WIRE_CORE_OBJECT_wire_types = "wire_types";

}  /* namespace  */

object_proxy::object_proxy(reference_ptr ref)
    : ref_(ref)
{
    if (!ref_)
        throw errors::runtime_error{ "Reference pointer is empty" };
}

bool
object_proxy::operator ==(object_proxy const& rhs) const
{
    return (!ref_ && ref_ == rhs.ref_)
            || (ref_ && rhs.ref_ && ref_->data() == rhs.ref_->data());
}

bool
object_proxy::operator <(object_proxy const& rhs) const
{
    return (!ref_ && rhs.ref_)
            || (ref_ && rhs.ref_ && ref_->data() < rhs.ref_->data());
}

::std::string const&
object_proxy::wire_static_type_id()
{
    return object::wire_static_type_id();
}

identity const&
object_proxy::wire_identity() const
{
    return wire_get_reference().object_id();
}

connection_ptr
object_proxy::wire_get_connection() const
{
    return wire_get_reference().get_connection();
}

connector_ptr
object_proxy::wire_get_connector() const
{
    return wire_get_reference().get_connector();
}

bool
object_proxy::wire_is_a(::std::string const& type_id, context_type const& ctx)
{
    auto future = wire_is_a_async(type_id, ctx, true);
    return future.get();
}

void
object_proxy::wire_is_a_async(::std::string const&  type_id,
        functional::callback< bool >                response,
        functional::exception_callback              exception,
        functional::callback< bool >                sent,
        context_type const&                         ctx,
        bool                                        run_sync)
{
    make_invocation(wire_get_reference(),
            WIRE_CORE_OBJECT_wire_is_a, ctx,
            &object::wire_is_a,
            response, exception, sent,
            type_id)(run_sync);
}

void
object_proxy::wire_ping(context_type const& ctx)
{
    auto future = wire_ping_async(ctx, true);
    future.get();
}

void
object_proxy::wire_ping_async(
        functional::void_callback        response,
        functional::exception_callback   exception,
        functional::callback< bool >     sent,
        context_type const&             ctx,
        bool                            run_sync
)
{
    make_invocation(wire_get_reference(),
            WIRE_CORE_OBJECT_wire_ping, ctx,
            &object::wire_ping,
            response, exception, sent)(run_sync);
}

::std::string
object_proxy::wire_type(context_type const& ctx)
{
    auto future = wire_type_async(ctx, true);
    return future.get();
}

void
object_proxy::wire_type_async(
        functional::callback< std::string const& >  response,
        functional::exception_callback              exception,
        functional::callback< bool >                sent,
        context_type const&                         ctx,
        bool                                        run_sync
)
{
    make_invocation(wire_get_reference(),
            WIRE_CORE_OBJECT_wire_type, ctx,
            &object::wire_type,
            response, exception, sent)(run_sync);
}

::std::vector< ::std::string >
object_proxy::wire_types(context_type const& ctx)
{
    auto future = wire_types_async(ctx, true);
    return future.get();
}

void
object_proxy::wire_types_async(
        functional::callback< ::std::vector< ::std::string > const& >   response,
        functional::exception_callback                                  exception,
        functional::callback<bool>                                      sent,
        context_type const&                                             ctx,
        bool                                                            run_sync
)
{
    make_invocation(wire_get_reference(),
            WIRE_CORE_OBJECT_wire_types, ctx,
            &object::wire_types,
            response, exception, sent)(run_sync);
}

::std::ostream&
operator << (::std::ostream& os, object_proxy const& val)
{
    ::std::ostream::sentry s (os);
    if (s) {
        os << val.wire_get_reference().data();
    }
    return os;
}


}  // namespace core
}  // namespace wire
