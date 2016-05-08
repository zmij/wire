/*
 * proxy.cpp
 *
 *  Created on: Jan 29, 2016
 *      Author: zmij
 */

#include <wire/core/proxy.hpp>
#include <wire/core/reference.hpp>
#include <wire/core/connection.hpp>

namespace wire {
namespace core {

bool
object_proxy::operator ==(object_proxy const& rhs) const
{
    return false;
}

bool
object_proxy::operator <(object_proxy const& rhs) const
{
    return false;
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

bool
object_proxy::wire_is_a(::std::string const& type_id, context_type const& ctx)
{
    auto future = wire_is_a_async(type_id, ctx);
    return future.get();
}

void
object_proxy::wire_is_a_async(::std::string const&  type_id,
        callbacks::callback< bool >                 response,
        callbacks::exception_callback               exception,
        callbacks::callback< bool >                 sent,
        context_type const&                         ctx,
        bool                                        run_sync)
{
    auto const& ref = wire_get_reference();
    wire_get_connection()->invoke(
        ref.object_id(), "wire_is_a", ctx, run_sync, response, exception, sent, type_id);
}

void
object_proxy::wire_ping(context_type const& ctx)
{
    auto future = wire_ping_async(ctx, true);
    future.get();
}

void
object_proxy::wire_ping_async(
        callbacks::void_callback        response,
        callbacks::exception_callback   exception,
        callbacks::callback< bool >     sent,
        context_type const&             ctx,
        bool                            run_sync
)
{
    auto const& ref = wire_get_reference();
    wire_get_connection()->invoke(
        ref.object_id(), "wire_ping", ctx, run_sync, response, exception, sent);
}

::std::string
object_proxy::wire_type(context_type const& ctx)
{
    auto future = wire_type_async(ctx, true);
    return std::move(future.get());
}

void
object_proxy::wire_type_async(
        callbacks::callback< std::string const& >   response,
        callbacks::exception_callback               exception,
        callbacks::callback< bool >                 sent,
        context_type const&                         ctx,
        bool                                        run_sync
)
{
    auto const& ref = wire_get_reference();
    wire_get_connection()->invoke(
        ref.object_id(), "wire_type", ctx, run_sync, response, exception, sent);
}

::std::vector< ::std::string >
object_proxy::wire_types(context_type const& ctx)
{
    auto future = wire_types_async(ctx, true);
    return future.get();
}

void
object_proxy::wire_types_async(
        callbacks::callback< ::std::vector< ::std::string > const& >    response,
        callbacks::exception_callback                                   exception,
        callbacks::callback<bool>                                       sent,
        context_type const&                                             ctx,
        bool                                                            run_sync
)
{
    auto const& ref = wire_get_reference();
    wire_get_connection()->invoke(
        ref.object_id(), "wire_types", ctx, run_sync, response, exception, sent);
}

}  // namespace core
}  // namespace wire
