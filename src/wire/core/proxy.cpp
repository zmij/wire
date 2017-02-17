/*
 * proxy.cpp
 *
 *  Created on: Jan 29, 2016
 *      Author: zmij
 */

#include <wire/core/proxy.hpp>
#include <wire/core/reference.hpp>
#include <wire/core/connection.hpp>
#include <wire/core/connector.hpp>
#include <wire/core/detail/configuration_options.hpp>
#include <wire/core/object.hpp>
#include <wire/core/invocation.hpp>

namespace wire {
namespace core {

namespace {

::std::string const WIRE_CORE_OBJECT_wire_is_a = "wire_is_a";
::std::string const WIRE_CORE_OBJECT_wire_ping = "wire_ping";
::std::string const WIRE_CORE_OBJECT_wire_type = "wire_type";
::std::string const WIRE_CORE_OBJECT_wire_types = "wire_types";

::std::uint32_t const WIRE_CORE_OBJECT_wire_is_a_hash = 0x1cdc304b;
::std::uint32_t const WIRE_CORE_OBJECT_wire_ping_hash = 0x7052047d;
::std::uint32_t const WIRE_CORE_OBJECT_wire_type_hash = 0x82d9cb3a;
::std::uint32_t const WIRE_CORE_OBJECT_wire_types_hash = 0x452e5af9;

}  /* namespace  */

object_proxy::object_proxy(reference_ptr ref, invocation_options const& opts)
    : ref_(ref), opts_(opts)
{
    if (!ref_)
        throw errors::runtime_error{ "Reference pointer is empty" };
    if (opts_ == invocation_options::unspecified) {
        opts_ = invocation_options(
            invocation_flags::none,
            ref->get_connector()->options().request_timeout);
    }
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

::std::string const&
object_proxy::wire_function_name(::std::uint32_t hash)
{
    return object::wire_function_name(hash);
}

identity const&
object_proxy::wire_identity() const
{
    return wire_get_reference()->object_id();
}

connection_ptr
object_proxy::wire_get_connection() const
{
    return wire_get_reference()->get_connection();
}

connector_ptr
object_proxy::wire_get_connector() const
{
    return wire_get_reference()->get_connector();
}

bool
object_proxy::wire_is_a(::std::string const& type_id, context_type const& ctx)
{
    auto future = wire_is_a_async(type_id, ctx,
            wire_invocation_options() | invocation_flags::sync);
    return future.get();
}

void
object_proxy::wire_is_a_async(::std::string const&  type_id,
        functional::callback< bool >                response,
        functional::exception_callback              exception,
        functional::callback< bool >                sent,
        context_type const&                         ctx,
        invocation_options                          opts)
{
    if (opts == invocation_options::unspecified)
        opts = wire_invocation_options();
    make_invocation(wire_get_reference(),
            WIRE_CORE_OBJECT_wire_is_a_hash, ctx,
            &object::wire_is_a,
            response, exception, sent,
            type_id)(opts);
}

void
object_proxy::wire_ping(context_type const& ctx)
{
    auto future = wire_ping_async(ctx,
            wire_invocation_options() | invocation_flags::sync);
    future.get();
}

void
object_proxy::wire_ping_async(
        functional::void_callback         response,
        functional::exception_callback    exception,
        functional::callback< bool >      sent,
        context_type const&               ctx,
        invocation_options                opts)
{
    if (opts == invocation_options::unspecified)
        opts = wire_invocation_options();
    make_invocation(wire_get_reference(),
            WIRE_CORE_OBJECT_wire_ping_hash, ctx,
            &object::wire_ping,
            response, exception, sent)(opts);
}

::std::string
object_proxy::wire_type(context_type const& ctx)
{
    auto future = wire_type_async(ctx,
            wire_invocation_options() | invocation_flags::sync);
    return future.get();
}

void
object_proxy::wire_type_async(
        functional::callback< std::string const& >  response,
        functional::exception_callback              exception,
        functional::callback< bool >                sent,
        context_type const&                         ctx,
        invocation_options                          opts)
{
    if (opts == invocation_options::unspecified)
        opts = wire_invocation_options();
    make_invocation(wire_get_reference(),
            WIRE_CORE_OBJECT_wire_type_hash, ctx,
            &object::wire_type,
            response, exception, sent)(opts);
}

::std::vector< ::std::string >
object_proxy::wire_types(context_type const& ctx)
{
    auto future = wire_types_async(ctx,
            wire_invocation_options() | invocation_flags::sync);
    return future.get();
}

void
object_proxy::wire_types_async(
        functional::callback< ::std::vector< ::std::string > const& >   response,
        functional::exception_callback                                  exception,
        functional::callback<bool>                                      sent,
        context_type const&                                             ctx,
        invocation_options                                              opts)
{
    if (opts == invocation_options::unspecified)
        opts = wire_invocation_options();
    make_invocation(wire_get_reference(),
            WIRE_CORE_OBJECT_wire_types_hash, ctx,
            &object::wire_types,
            response, exception, sent)(opts);
}

object_prx
object_proxy::wire_well_known_proxy() const
{
    auto const& ref = ref_->data();
    return ::std::make_shared< object_proxy >(
            reference::create_reference(wire_get_connector(),
                    { ref.object_id, ref.facet }), opts_);
}

object_prx
object_proxy::wire_with_identity(identity const& id) const
{
    auto const& ref = ref_->data();
    return ::std::make_shared< object_proxy >(
            reference::create_reference(wire_get_connector(),
                    { id, ref.facet, ref.adapter, ref.endpoints}), opts_);
}

object_prx
object_proxy::wire_with_endpoints(endpoint_list const& eps) const
{
    auto const& ref = ref_->data();
    return ::std::make_shared< object_proxy >(
            reference::create_reference(wire_get_connector(),
                    { ref.object_id, ref.facet, ref.adapter, eps}), opts_);
}

object_prx
object_proxy::wire_one_way() const
{
    return ::std::make_shared< object_proxy >(ref_,
            opts_ | invocation_flags::one_way);
}

object_prx
object_proxy::wire_timeout(invocation_options::timeout_type t) const
{
    return ::std::make_shared< object_proxy >( ref_, opts_.with_timeout(t));
}

::std::ostream&
operator << (::std::ostream& os, object_proxy const& val)
{
    ::std::ostream::sentry s (os);
    if (s) {
        os << val.wire_get_reference()->data();
    }
    return os;
}


}  // namespace core
}  // namespace wire
