/*
 * connection.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_CONNECTION_HPP_
#define WIRE_CORE_CONNECTION_HPP_

#include <wire/asio_config.hpp>

#include <wire/core/connection_fwd.hpp>

#include <wire/core/endpoint.hpp>
#include <wire/core/identity.hpp>
#include <wire/core/context.hpp>

#include <wire/core/connector_fwd.hpp>
#include <wire/core/object_fwd.hpp>
#include <wire/core/adapter_fwd.hpp>
#include <wire/core/functional.hpp>
#include <wire/core/connection_observer_fwd.hpp>

#include <wire/core/detail/dispatch_request_fwd.hpp>

#include <pushkin/meta/function_traits.hpp>

#include <wire/encoding/buffers.hpp>
#include <wire/encoding/message.hpp>

#include <tuple>

namespace wire {
namespace core {

struct server_side {};
struct client_side {};

namespace detail {

struct connection_implementation;
using connection_impl_ptr = ::std::shared_ptr<connection_implementation>;
}  /* namespace detail */

class connection {
public:
    // FIXME Change callback signature to (endpoint, pointer)
    using close_callback = functional::callback<connection const*>;
public:
    /**
     * Create connection with no endpoint.
     */
    connection(client_side const&, adapter_ptr, transport_type, close_callback);
    /**
     * Create connection and start asynchronous connect.
     * @param endpoint
     * @param cb Connected callback
     * @param ecb Exception callback
     * @param
     */
    connection(client_side const&, adapter_ptr, endpoint const&,
            functional::void_callback       on_connect  = nullptr,
            functional::exception_callback  on_error    = nullptr,
            close_callback                  on_close    = nullptr);

    /**
     * Create a connection and start accepting at specified
     * endpoint
     * @param adapter shared pointer to adapter
     * @param endpoint endpoint to listen
     */
    connection(server_side const&, adapter_ptr, endpoint const&);

    ~connection();

    connector_ptr
    get_connector() const;

    void
    add_observer(connection_observer_ptr);
    void
    remove_observer(connection_observer_ptr);
    /**
     * Start asynchronous connect
     * @param endpoint
     * @param cb Connected callback
     * @param ecb Exception callback
     */
    void
    connect_async(endpoint const&,
            functional::void_callback       on_connect  = nullptr,
            functional::exception_callback  on_error    = nullptr);


    void
    close();

    void
    set_adapter(adapter_ptr);

    template < typename Handler, typename ... Args >
    typename ::std::enable_if< (::psst::meta::is_callable_object<Handler>::value &&
            ::psst::meta::function_traits< Handler >::arity > 0), void >::type
    invoke(encoding::invocation_target const& target,
            encoding::operation_specs::operation_id const& op,
            context_type const& ctx,
            invocation_options const& opts,
            Handler response,
            functional::exception_callback exception,
            functional::callback< bool > sent,
            Args const& ... args)
    {
        using handler_traits = ::psst::meta::function_traits<Handler>;
        using args_tuple = typename handler_traits::decayed_args_tuple_type;

        using encoding::incoming;
        if (any(opts.flags & invocation_flags::one_way)) {
            throw errors::runtime_error{ "Cannot send a non-void invocation one way" };
        }
        encoding::outgoing out{ get_connector() };
        encoding::write(::std::back_inserter(out), args ...);
        invoke(target, op, ctx, opts, ::std::move(out),
            [response, exception](incoming::const_iterator begin, incoming::const_iterator end){
                try {
                    auto encaps = begin.incoming_encapsulation();
                    args_tuple args;
                    encoding::read(begin, end, args);
                    encaps.read_indirection_table(begin);
                    ::psst::meta::invoke(response, args);
                } catch(...) {
                    if (exception) {
                        try {
                            exception(::std::current_exception());
                        } catch (...) {}
                    }
                }
            },
            exception, sent);
    }

    template < typename ... Args >
    void
    invoke(encoding::invocation_target const& target,
            encoding::operation_specs::operation_id const& op,
            context_type const& ctx,
            invocation_options const&        opts,
            functional::void_callback        response,
            functional::exception_callback   exception,
            functional::callback< bool >     sent,
            Args&& ... args)
    {
        using encoding::incoming;
        encoding::outgoing out{ get_connector() };
        write(::std::back_inserter(out), ::std::forward<Args>(args) ...);
        invoke(target, op, ctx, opts, ::std::move(out),
            [response, exception](incoming::const_iterator, incoming::const_iterator){
                if (response) {
                    try {
                        response();
                    } catch(...) {
                        if (exception) {
                            try {
                                exception(::std::current_exception());
                            } catch(...) {}
                        }
                    }
                }
            },
            exception, sent);
    }

    void
    invoke(encoding::invocation_target const&,
            encoding::operation_specs::operation_id const& op,
            context_type const& ctx,
            invocation_options const&,
            encoding::outgoing&&,
            encoding::reply_callback,
            functional::exception_callback exception,
            functional::callback< bool > sent);

    template < typename ... Args >
    void
    send(encoding::multiple_targets const& targets,
            encoding::operation_specs::operation_id const& op,
            context_type const& ctx,
            invocation_options const& opts,
            functional::exception_callback exception,
            functional::callback< bool > sent,
            Args&& ... args)
    {
        using encoding::incoming;
        encoding::outgoing out{ get_connector() };
        write(::std::back_inserter(out), ::std::forward<Args>(args) ...);
        send(targets, op, ctx, opts, ::std::move(out), exception, sent);
    }

    void
    send(encoding::multiple_targets const&,
            encoding::operation_specs::operation_id const& op,
            context_type const& ctx,
            invocation_options const&,
            encoding::outgoing&&,
            functional::exception_callback exception,
            functional::callback< bool > sent);

    void
    forward(encoding::multiple_targets const&,
            encoding::operation_specs::operation_id const& op,
            context_type const& ctx,
            invocation_options const& opts,
            detail::dispatch_request const& req,
            encoding::reply_callback reply,
            functional::exception_callback exception,
            functional::callback< bool > sent);

    endpoint
    local_endpoint() const;
    endpoint
    remote_endpoint() const;
private:
    connection(connection&&) = delete;
    connection(connection const&) = delete;
    connection&
    operator = (connection&&) = delete;
    connection&
    operator = (connection const&) = delete;

    void
    create_client_connection(adapter_ptr, transport_type, close_callback);
private:
//    struct impl;
//    using pimpl = ::std::unique_ptr<impl>;
    detail::connection_impl_ptr pimpl_;
};

}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_CONNECTION_HPP_ */
