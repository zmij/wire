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

#include <wire/util/function_traits.hpp>

#include <wire/encoding/buffers.hpp>

#include <tuple>

namespace wire {
namespace core {

class connection {
public:
    /**
     * Create connection with no endpoint.
     */
    connection(connector_ptr cnctr, asio_config::io_service_ptr);
    /**
     * Create connection and start asynchronous connect.
     * @param
     */
    connection(connector_ptr cnctr, asio_config::io_service_ptr, endpoint const&,
            functional::void_callback = nullptr,
            functional::exception_callback = nullptr);

    /**
     * Create a connection and start accepting at specified
     * endpoint
     * @param adapter shared pointer to adapter
     * @param endpoint endpoint to listen
     */
    connection(adapter_ptr, endpoint const&);

    connection(connection&&);
    connection&
    operator = (connection&&);

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
            functional::void_callback = nullptr,
            functional::exception_callback = nullptr);


    void
    close();

    void
    set_adapter(adapter_ptr);

    template < typename Handler, typename ... Args >
    typename std::enable_if< (util::is_callable<Handler>::value &&
            util::function_traits< Handler >::arity > 0), void >::type
    invoke(identity const& id, std::string const& op, context_type const& ctx,
            bool run_sync,
            Handler response,
            functional::exception_callback exception,
            functional::callback< bool > sent,
            Args const& ... args)
    {
        using handler_traits = util::function_traits<Handler>;
        using args_tuple = typename handler_traits::decayed_args_tuple_type;

        using encoding::incoming;
        encoding::outgoing out{ get_connector() };
        encoding::write(std::back_inserter(out), args ...);
        invoke(id, op, ctx, run_sync, ::std::move(out),
            [response, exception](incoming::const_iterator begin, incoming::const_iterator end){
                try {
                    auto encaps = begin.incoming_encapsulation();
                    args_tuple args;
                    encoding::read(begin, end, args);
                    encaps.read_indirection_table(begin);
                    util::invoke(response, args);
                } catch(...) {
                    if (exception) {
                        exception(std::current_exception());
                    }
                }
            },
            exception, sent);
    }

    template < typename ... Args >
    void
    invoke(identity const& id, std::string const& op, context_type const& ctx,
            bool run_sync,
            functional::void_callback        response,
            functional::exception_callback   exception,
            functional::callback< bool >     sent,
            Args const& ... args)
    {
        using encoding::incoming;
        encoding::outgoing out{ get_connector() };
        write(std::back_inserter(out), args ...);
        invoke(id, op, ctx, run_sync, ::std::move(out),
            [response](incoming::const_iterator, incoming::const_iterator){
                if (response) {
                    response();
                }
            },
            exception, sent);
    }

    void
    invoke(identity const&, std::string const& op, context_type const& ctx,
            bool run_sync,
            encoding::outgoing&&,
            encoding::reply_callback,
            functional::exception_callback exception,
            functional::callback< bool > sent);

    endpoint
    local_endpoint() const;
private:
    connection(connection const&) = delete;
    connection&
    operator = (connection const&) = delete;
private:
    struct impl;
    using pimpl = std::shared_ptr<impl>;
    pimpl pimpl_;
};

}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_CONNECTION_HPP_ */
