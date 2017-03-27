/*
 * transport.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_TRANSPORT_HPP_
#define WIRE_CORE_TRANSPORT_HPP_

#include <wire/asio_ssl_config.hpp>
#include <wire/future_config.hpp>
#include <wire/core/endpoint.hpp>
#include <wire/core/detail/ssl_options.hpp>

#include <pushkin/asio/async_ssl_ops.hpp>

#include <memory>
#include <future>
#include <atomic>

namespace wire {
namespace core {

#ifdef SO_REUSEPORT
typedef asio_ns::detail::socket_option::boolean<
BOOST_ASIO_OS_DEF(SOL_SOCKET), SO_REUSEPORT > reuse_port;
#endif

struct tcp_transport;
struct ssl_transport;
struct udp_transport;
struct socket_transport;

template<transport_type>
struct transport_type_traits;

template<>
struct transport_type_traits<transport_type::tcp> {
    static constexpr transport_type value   = transport_type::tcp;
    static constexpr bool stream_oriented   = true;
    static constexpr bool is_ip             = true;

    using type = tcp_transport;
    using endpoint_data         = detail::tcp_endpoint_data;

    using protocol              = asio_config::tcp;
    using socket_type           = protocol::socket;
    using listen_socket_type    = protocol::socket;
    using endpoint_type         = protocol::endpoint;
    using resolver_type         = protocol::resolver;
    using acceptor_type         = protocol::acceptor;

    static endpoint_type
    create_endpoint(asio_config::io_service_ptr svc, endpoint const&);
    static endpoint
    get_endpoint_data(endpoint_type const&);

    static void
    close_acceptor(acceptor_type& acceptor)
    {
        acceptor.cancel();
        acceptor.close();
    }
};

template<>
struct transport_type_traits<transport_type::ssl> {
    static constexpr transport_type value   = transport_type::ssl;
    static constexpr bool stream_oriented   = true;
    static constexpr bool is_ip             = true;

    using type                  = ssl_transport;
    using endpoint_data         = detail::ssl_endpoint_data;

    using protocol              = asio_config::tcp;
    using socket_type           = asio_ns::ssl::stream< protocol::socket >;
    using listen_socket_type    = socket_type::lowest_layer_type;
    using endpoint_type         = protocol::endpoint;
    using resolver_type         = protocol::resolver;
    using acceptor_type         = protocol::acceptor;

    static endpoint_type
    create_endpoint(asio_config::io_service_ptr svc, endpoint const&);
    static endpoint
    get_endpoint_data(endpoint_type const&);

    static void
    close_acceptor(acceptor_type& acceptor)
    {
        acceptor.cancel();
        acceptor.close();
    }
};

template<>
struct transport_type_traits<transport_type::udp> {
    static constexpr transport_type value   = transport_type::udp;
    static constexpr bool stream_oriented   = false;
    static constexpr bool is_ip             = true;

    using type                  = udp_transport;
    using endpoint_data         = detail::udp_endpoint_data;

    using protocol              = asio_config::udp;
    using socket_type           = protocol::socket;
    using listen_socket_type    = protocol::socket;
    using endpoint_type         = protocol::endpoint;
    using resolver_type         = protocol::resolver;

    static endpoint_type
    create_endpoint(asio_config::io_service_ptr svc, endpoint const&);
    static endpoint
    get_endpoint_data(endpoint_type const&);
};

template<>
struct transport_type_traits<transport_type::socket> {
    static constexpr transport_type value   = transport_type::socket;
    static constexpr bool stream_oriented   = true;
    static constexpr bool is_ip             = false;

    using type                  = socket_transport;
    using endpoint_data         = detail::socket_endpoint_data;

    using protocol              = asio_config::local_socket;
    using socket_type           = protocol::socket;
    using listen_socket_type    = protocol::socket;
    using endpoint_type         = protocol::endpoint;
    using acceptor_type         = protocol::acceptor;

    static endpoint_type
    create_endpoint(asio_config::io_service_ptr svc, endpoint const&);
    static endpoint
    get_endpoint_data(endpoint_type const&);

    static void
    close_acceptor(acceptor_type& acceptor);
};

struct tcp_transport {
    using traits                = transport_type_traits< transport_type::tcp >;
    using socket_type           = traits::socket_type;
    using listen_socket_type    = traits::listen_socket_type;
    using resolver_type         = traits::resolver_type;
    using strand_type           = asio_config::io_service::strand;
    static constexpr transport_type type = transport_type::tcp;

    tcp_transport(asio_config::io_service_ptr);

    void
    connect(endpoint const& ep);
    void
    connect_async(endpoint const& ep, asio_config::asio_callback);
    template < template < typename > class _Promise = promise >
    auto
    connect_async(endpoint const& ep)
        -> decltype(::std::declval<_Promise<void>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<void> >();
        connect_async(ep,
            [promise](asio_config::error_code const& ec)
            {
                if (!ec) {
                    promise->set_value();
                } else {
                    promise->set_exception(
                        ::std::make_exception_ptr(::boost::system::system_error{ec}) );
                }
            });

        return promise->get_future();
    }
    void
    close();

    bool
    is_open() const
    {
        return socket_.is_open();
    }

    template<typename BufferType, typename HandlerType>
    void
    async_write(BufferType const& buffer, HandlerType handler)
    {
        if (socket_.is_open()) {
            ::psst::asio::async_write(socket_, buffer, ::std::move(handler));
        } else {
            handler(asio_config::make_error_code( asio_config::error::shut_down ), 0);
        }
    }

    template < typename BufferType, template < typename > class _Promise = promise >
    auto
    async_write(BufferType const& buffer)
        -> decltype(::std::declval<_Promise<::std::size_t>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<::std::size_t> >();
        async_write(buffer,
            [promise](asio_config::error_code const& ec, ::std::size_t bytes)
            {
                if (!ec) {
                    promise->set_value(bytes);
                } else {
                    promise->set_exception(
                        ::std::make_exception_ptr(::boost::system::system_error{ec}) );
                }
            });
        return promise->get_future();
    }

    template < typename BufferType, typename HandlerType >
    void
    async_read(BufferType&& buffer, HandlerType handler)
    {
        if (socket_.is_open()) {
            ::psst::asio::async_read(socket_, ::std::forward<BufferType>(buffer),
                    ::std::move(handler));
        } else {
            handler(asio_config::make_error_code( asio_config::error::shut_down ), 0);
        }
    }

    template < typename BufferType, template < typename > class _Promise = promise >
    auto
    async_read(BufferType&& buffer)
        -> decltype(::std::declval<_Promise<::std::size_t>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<::std::size_t> >();
        async_read(::std::forward<BufferType>(buffer),
            [promise](asio_config::error_code const& ec, ::std::size_t bytes)
            {
                if (!ec) {
                    promise->set_value(bytes);
                } else {
                    promise->set_exception(
                        ::std::make_exception_ptr(::boost::system::system_error{ec}) );
                }
            });
        return promise->get_future();
    }

    listen_socket_type&
    socket()
    {   return socket_;}
    listen_socket_type const&
    socket() const
    {   return socket_;}

    endpoint
    local_endpoint() const
    {
        return traits::get_endpoint_data(socket_.local_endpoint());
    }
    endpoint
    local_endpoint(asio_config::error_code& ec) const
    {
        auto ep = socket_.local_endpoint(ec);
        if (ec) {
            return endpoint{};
        }
        return traits::get_endpoint_data(ep);
    }

    endpoint
    remote_endpoint() const
    {
        return traits::get_endpoint_data(socket_.remote_endpoint());
    }
    endpoint
    remote_endpoint(asio_config::error_code& ec) const
    {
        auto ep = socket_.remote_endpoint(ec);
        if (ec) {
            return endpoint{};
        }
        return traits::get_endpoint_data(ep);
    }
private:
    void
    handle_resolve(asio_config::error_code const& ec,
            resolver_type::iterator endpoint_iterator,
            asio_config::asio_callback);
    void
    handle_connect(asio_config::error_code const& ec, asio_config::asio_callback);
private:
    tcp_transport(tcp_transport const&) = delete;
    tcp_transport&
    operator = (tcp_transport const&) = delete;
private:
    resolver_type   resolver_;
    socket_type     socket_;
    // TODO timeout settings
};

struct ssl_transport {
    using traits                = transport_type_traits< transport_type::ssl >;
    using socket_type           = traits::socket_type;
    using listen_socket_type    = traits::listen_socket_type;
    using resolver_type         = traits::resolver_type;
    using verify_mode           = asio_ns::ssl::verify_mode;
    static constexpr transport_type type = transport_type::ssl;

    static asio_config::ssl_context
    create_context(detail::ssl_options const&);

    ssl_transport(asio_config::io_service_ptr, detail::ssl_options const& =
            detail::ssl_options { });

    /**
     * Client connect.
     * @param ep endpoint to connect to
     * @param cb callback that is called when the operation finishes
     */
    void
    connect_async(endpoint const& ep, asio_config::asio_callback);

    /**
     * Server start.
     * @param cb callback that is called when the operation finishes
     */
    void
    start(asio_config::asio_callback);

    void set_verify_mode(verify_mode mode)
    {
        verify_mode_ = mode;
    }

    void
    close();

    bool
    is_open() const
    {
        return socket_.lowest_layer().is_open();
    }

    template<typename BufferType, typename HandlerType>
    void async_write(BufferType const& buffer, HandlerType handler)
    {
        if (socket_.lowest_layer().is_open()) {
            ::psst::asio::async_write(socket_, buffer, ::std::move(handler));
        } else {
            handler(asio_config::make_error_code( asio_config::error::shut_down ), 0);
        }
    }

    template < typename BufferType, template < typename > class _Promise = promise >
    auto
    async_write(BufferType const& buffer)
        -> decltype(::std::declval<_Promise<::std::size_t>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<::std::size_t> >();
        async_write(buffer,
            [promise](asio_config::error_code const& ec, ::std::size_t bytes)
            {
                if (!ec) {
                    promise->set_value(bytes);
                } else {
                    promise->set_exception(
                        ::std::make_exception_ptr(::boost::system::system_error{ec}) );
                }
            });
        return promise->get_future();
    }

    template < typename BufferType, typename HandlerType >
    void
    async_read(BufferType&& buffer, HandlerType handler)
    {
        if (socket_.lowest_layer().is_open()) {
            ::psst::asio::async_read(socket_, ::std::forward<BufferType>(buffer),
                    ::std::move(handler));
        } else {
            handler(asio_config::make_error_code( asio_config::error::shut_down ), 0);
        }
    }

    template < typename BufferType, template < typename > class _Promise = promise >
    auto
    async_read(BufferType&& buffer)
        -> decltype(::std::declval<_Promise<::std::size_t>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<::std::size_t> >();
        async_read(::std::forward<BufferType>(buffer),
            [promise](asio_config::error_code const& ec, ::std::size_t bytes)
            {
                if (!ec) {
                    promise->set_value(bytes);
                } else {
                    promise->set_exception(
                        ::std::make_exception_ptr(::boost::system::system_error{ec}) );
                }
            });
        return promise->get_future();
    }

    listen_socket_type&
    socket()
    {   return socket_.lowest_layer();}
    listen_socket_type const&
    socket() const
    {   return socket_.lowest_layer();}

    endpoint
    local_endpoint() const
    {
        return traits::get_endpoint_data(socket_.lowest_layer().local_endpoint() );
    }
    endpoint
    local_endpoint(asio_config::error_code& ec) const
    {
        auto ep = socket_.lowest_layer().local_endpoint(ec);
        if (ec) {
            return endpoint{};
        }
        return traits::get_endpoint_data(ep);
    }

    endpoint
    remote_endpoint() const
    {
        return traits::get_endpoint_data(socket_.lowest_layer().remote_endpoint());
    }
    endpoint
    remote_endpoint(asio_config::error_code& ec) const
    {
        auto ep = socket_.lowest_layer().remote_endpoint(ec);
        if (ec) {
            return endpoint{};
        }
        return traits::get_endpoint_data(ep);
    }
private:
    bool
    verify_certificate(bool preverified, asio_ns::ssl::verify_context& ctx);

    void
    handle_resolve(asio_config::error_code const& ec,
            resolver_type::iterator endpoint_iterator,
            asio_config::asio_callback);
    void
    handle_connect(asio_config::error_code const& ec, asio_config::asio_callback);
    void
    handle_handshake(asio_config::error_code const& ec, asio_config::asio_callback);
private:
    ssl_transport(ssl_transport const&) = delete;
    ssl_transport&
    operator = (ssl_transport const&) = delete;
private:
    asio_config::ssl_context    ctx_;
    resolver_type               resolver_;
    socket_type                 socket_;
    verify_mode                 verify_mode_ = asio_ns::ssl::verify_peer;
};

struct udp_transport {
    using traits                = transport_type_traits< transport_type::udp >;
    using protocol              = traits::protocol;
    using socket_type           = traits::socket_type;
    using endpoint_type         = traits::endpoint_type;
    using resolver_type         = traits::resolver_type;
    static constexpr transport_type type = transport_type::udp;

    udp_transport(asio_config::io_service_ptr);
    /**
     * Client connect.
     * @param ep endpoint to connect to
     * @param cb callback that is called when the operation finishes
     */
    void
    connect_async(endpoint const& ep, asio_config::asio_callback);

    void
    close();

    bool
    is_open() const
    {
        return socket_.is_open();
    }

    template<typename BufferType, typename HandlerType>
    void async_write(BufferType const& buffer, HandlerType handler)
    {
        if (socket_.is_open()) {
            socket_.async_send(buffer, handler);
        } else {
            handler(asio_config::make_error_code( asio_config::error::shut_down ), 0);
        }
    }
    template<typename BufferType, typename HandlerType>
    void async_read(BufferType& buffer, HandlerType handler)
    {
        if (socket_.is_open()) {
            socket_.async_receive(buffer, handler);
        } else {
            handler(asio_config::make_error_code( asio_config::error::shut_down ), 0);
        }
    }

    endpoint
    local_endpoint() const
    {
        return traits::get_endpoint_data(socket_.local_endpoint());
    }
    endpoint
    local_endpoint(asio_config::error_code& ec) const
    {
        auto ep = socket_.local_endpoint(ec);
        if (ec) {
            return endpoint{};
        }
        return traits::get_endpoint_data(ep);
    }

    endpoint
    remote_endpoint() const
    {
        return traits::get_endpoint_data(socket_.remote_endpoint());
    }
    endpoint
    remote_endpoint(asio_config::error_code& ec) const
    {
        auto ep = socket_.remote_endpoint(ec);
        if (ec) {
            return endpoint{};
        }
        return traits::get_endpoint_data(ep);
    }
private:
    void
    handle_resolve(asio_config::error_code const& ec,
            resolver_type::iterator endpoint_iterator,
            asio_config::asio_callback);
    void
    handle_connect(asio_config::error_code const&, asio_config::asio_callback);
private:
    udp_transport(udp_transport const&) = delete;
    udp_transport&
    operator =(udp_transport const&) = delete;
protected:
    resolver_type   resolver_;
    socket_type     socket_;
};

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
struct socket_transport {
    using traits = transport_type_traits< transport_type::socket >;
    using socket_type = traits::socket_type;
    using listen_socket_type    = traits::listen_socket_type;
    using endpoint_type = traits::endpoint_type;
    static constexpr transport_type type = transport_type::socket;

    socket_transport(asio_config::io_service_ptr);
    /**
     * Client connect.
     * @param ep endpoint to connect to
     * @param cb callback that is called when the operation finishes
     */
    void
    connect_async(endpoint const& ep, asio_config::asio_callback);

    void
    close();

    bool
    is_open() const
    {
        return socket_.is_open();
    }

    template<typename BufferType, typename HandlerType>
    void async_write(BufferType const& buffer, HandlerType handler)
    {
        if (socket_.is_open()) {
            ::psst::asio::async_write(socket_, buffer, ::std::move(handler));
        } else {
            handler(asio_config::make_error_code( asio_config::error::shut_down ), 0);
        }
    }

    template < typename BufferType, template < typename > class _Promise = promise >
    auto
    async_write(BufferType const& buffer)
        -> decltype(::std::declval<_Promise<::std::size_t>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<::std::size_t> >();
        async_write(buffer,
            [promise](asio_config::error_code const& ec, ::std::size_t bytes)
            {
                if (!ec) {
                    promise->set_value(bytes);
                } else {
                    promise->set_exception(
                        ::std::make_exception_ptr(::boost::system::system_error{ec}) );
                }
            });
        return promise->get_future();
    }

    template < typename BufferType, typename HandlerType >
    void
    async_read(BufferType&& buffer, HandlerType handler)
    {
        if (socket_.is_open()) {
            ::psst::asio::async_read(socket_, ::std::forward< BufferType >(buffer),
                    ::std::move(handler));
        } else {
            handler(asio_config::make_error_code( asio_config::error::shut_down ), 0);
        }
    }

    template < typename BufferType, template < typename > class _Promise = promise >
    auto
    async_read(BufferType&& buffer)
        -> decltype(::std::declval<_Promise<::std::size_t>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<::std::size_t> >();
        async_read(::std::forward<BufferType>(buffer),
            [promise](asio_config::error_code const& ec, ::std::size_t bytes)
            {
                if (!ec) {
                    promise->set_value(bytes);
                } else {
                    promise->set_exception(
                        ::std::make_exception_ptr(::boost::system::system_error{ec}) );
                }
            });
        return promise->get_future();
    }

    listen_socket_type&
    socket()
    {   return socket_;}
    listen_socket_type const&
    socket() const
    {   return socket_;}

    endpoint
    local_endpoint() const
    {
        return traits::get_endpoint_data(socket_.local_endpoint());
    }
    endpoint
    local_endpoint(asio_config::error_code& ec) const
    {
        auto ep = socket_.local_endpoint(ec);
        if (ec) {
            return endpoint{};
        }
        return traits::get_endpoint_data(ep);
    }

    endpoint
    remote_endpoint() const
    {
        return traits::get_endpoint_data(socket_.remote_endpoint());
    }
    endpoint
    remote_endpoint(asio_config::error_code& ec) const
    {
        auto ep = socket_.remote_endpoint(ec);
        if (ec) {
            return endpoint{};
        }
        return traits::get_endpoint_data(ep);
    }
private:
    void
    handle_connect(asio_config::error_code const&, asio_config::asio_callback);
private:
    socket_transport(socket_transport const&) = delete;
    socket_transport&
    operator = (socket_transport const&) = delete;
private:
    socket_type socket_;
};
#endif /* BOOST_ASIO_HAS_LOCAL_SOCKETS */

template<typename Session, transport_type Type>
struct transport_listener {
    using traits                = transport_type_traits< Type >;
    using acceptor_type         = typename traits::acceptor_type;
    using endpoint_type         = typename traits::endpoint_type;
    using endpoint_data         = typename traits::endpoint_data;
    using session_type          = Session;
    using session_ptr           = std::shared_ptr<Session>;
    using session_factory       = std::function< session_ptr(asio_config::io_service_ptr) >;

    transport_listener(asio_config::io_service_ptr, session_factory);
    ~transport_listener();

    void
    open(endpoint const&, bool reuse_port = false);

    void
    close();

    endpoint
    local_endpoint() const;

    endpoint
    remote_endpoint() const
    { return endpoint{}; }

    bool
    ready() const
    { return ready_; }

    bool
    is_open() const
    { return acceptor_.is_open(); }
private:
    void
    start_accept();

    session_ptr
    create_session();
    void
    handle_accept(session_ptr session, asio_config::error_code const& ec);
private:
    transport_listener(transport_listener const&) = delete;
    transport_listener&
    operator =(transport_listener const&) = delete;
private:
    asio_config::io_service_ptr io_service_;
    acceptor_type               acceptor_;
    session_factory             factory_;
    ::std::atomic<bool>         ready_;
    ::std::atomic<bool>         closed_;
};

template<>
struct transport_listener<void, transport_type::udp> : udp_transport {
    using traits                = transport_type_traits< transport_type::udp >;
    using endpoint_type         = traits::endpoint_type;
    using endpoint_data         = traits::endpoint_data;

    transport_listener(asio_config::io_service_ptr);

    void
    open(endpoint const&);

    endpoint
    local_endpoint() const;
private:
    transport_listener(transport_listener const&) = delete;
    transport_listener&
    operator =(transport_listener const&) = delete;
private:
    asio_config::io_service_ptr io_service_;
};

}  // namespace core
}  // namespace wire

#include <wire/core/transport.inl>

#endif /* WIRE_CORE_TRANSPORT_HPP_ */
