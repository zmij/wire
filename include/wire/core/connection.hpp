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

#include <pushkin/meta/function_traits.hpp>

#include <wire/encoding/buffers.hpp>

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
    invoke(identity const& id, ::std::string const& op, context_type const& ctx,
            bool run_sync,
            Handler response,
            functional::exception_callback exception,
            functional::callback< bool > sent,
            Args const& ... args)
    {
        using handler_traits = ::psst::meta::function_traits<Handler>;
        using args_tuple = typename handler_traits::decayed_args_tuple_type;

        using encoding::incoming;
        encoding::outgoing out{ get_connector() };
        encoding::write(::std::back_inserter(out), args ...);
        invoke(id, op, ctx, run_sync, ::std::move(out),
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
    invoke(identity const& id, ::std::string const& op, context_type const& ctx,
            bool run_sync,
            functional::void_callback        response,
            functional::exception_callback   exception,
            functional::callback< bool >     sent,
            Args const& ... args)
    {
        using encoding::incoming;
        encoding::outgoing out{ get_connector() };
        write(::std::back_inserter(out), args ...);
        invoke(id, op, ctx, run_sync, ::std::move(out),
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
    invoke(identity const&, ::std::string const& op, context_type const& ctx,
            bool run_sync,
            encoding::outgoing&&,
            encoding::reply_callback,
            functional::exception_callback exception,
            functional::callback< bool > sent);

    endpoint
    local_endpoint() const;
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
