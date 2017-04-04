/*
 * transport.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#include <wire/core/transport.hpp>
#include <wire/errors/exceptions.hpp>

#include <iostream>

namespace wire {
namespace core {

constexpr transport_type transport_type_traits< transport_type::tcp >::value;
constexpr bool transport_type_traits< transport_type::tcp >::stream_oriented;

constexpr transport_type transport_type_traits< transport_type::ssl >::value;
constexpr bool transport_type_traits< transport_type::ssl >::stream_oriented;

constexpr transport_type transport_type_traits< transport_type::udp >::value;
constexpr bool transport_type_traits< transport_type::udp >::stream_oriented;

constexpr transport_type transport_type_traits< transport_type::socket >::value;
constexpr bool transport_type_traits< transport_type::socket >::stream_oriented;

//----------------------------------------------------------------------------
//    Transport traits implementation
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//    TCP transport traits implementation
//----------------------------------------------------------------------------
transport_type_traits< transport_type::tcp >::endpoint_type
transport_type_traits< transport_type::tcp >::create_endpoint(
        asio_config::io_service_ptr svc, endpoint const& ep)
{
    // TODO System-assigned endpoint from an empty enpoint
    endpoint_data const& ed = ep.get< endpoint_data >();
    resolver_type resolver(*svc);
    resolver_type::query query(ed.host, ::std::to_string(ed.port));
    endpoint_type proto_ep = *resolver.resolve(query);
    return proto_ep;
}

endpoint
transport_type_traits< transport_type::tcp >::get_endpoint_data(endpoint_type const& ep)
{
    return endpoint::tcp(ep.address().to_string(), ep.port());
}

//----------------------------------------------------------------------------
//    SSL transport traits implementation
//----------------------------------------------------------------------------
transport_type_traits< transport_type::ssl >::endpoint_type
transport_type_traits< transport_type::ssl >::create_endpoint(
        asio_config::io_service_ptr svc, endpoint const& ep)
{
    // TODO System-assigned endpoint from an empty enpoint
    endpoint_data const& ed = ep.get< endpoint_data >();
    resolver_type resolver(*svc);
    resolver_type::query query(ed.host, ::std::to_string(ed.port));
    endpoint_type proto_ep = *resolver.resolve(query);
    return proto_ep;
}

endpoint
transport_type_traits< transport_type::ssl >::get_endpoint_data(endpoint_type const& ep)
{
    return endpoint::ssl(ep.address().to_string(), ep.port());
}

transport_type_traits< transport_type::udp >::endpoint_type
transport_type_traits< transport_type::udp >::create_endpoint(
        asio_config::io_service_ptr svc, endpoint const& ep)
{
    // TODO System-assigned endpoint from an empty enpoint
    endpoint_data const& ed = ep.get< endpoint_data >();
    resolver_type resolver(*svc);
    resolver_type::query query(ed.host, ::std::to_string(ed.port));
    endpoint_type proto_ep = *resolver.resolve(query);
    return proto_ep;
}

//----------------------------------------------------------------------------
//    UDP transport traits implementation
//----------------------------------------------------------------------------
endpoint
transport_type_traits< transport_type::udp >::get_endpoint_data(endpoint_type const& ep)
{
    return endpoint::udp(ep.address().to_string(), ep.port());
}

//----------------------------------------------------------------------------
//    Socket transport traits implementation
//----------------------------------------------------------------------------
transport_type_traits< transport_type::socket >::endpoint_type
transport_type_traits< transport_type::socket >::create_endpoint(
        asio_config::io_service_ptr svc, endpoint const& ep)
{
    // TODO System-assigned endpoint from an empty enpoint
    endpoint_data const& ed = ep.get< endpoint_data >();
    return endpoint_type{ ed.path };
}

endpoint
transport_type_traits< transport_type::socket >::get_endpoint_data(endpoint_type const& ep)
{
    return endpoint::socket(ep.path());
}

void
transport_type_traits< transport_type::socket >::close_acceptor(acceptor_type& acceptor)
{
    auto ep = acceptor.local_endpoint();
    auto path = ep.path();
    acceptor.cancel();
    acceptor.close();
    unlink(path.c_str());
}

//----------------------------------------------------------------------------
//    TCP transport
//----------------------------------------------------------------------------
tcp_transport::tcp_transport(asio_config::io_service_ptr io_svc)
    : resolver_{*io_svc}, socket_{*io_svc}
{
}

void
tcp_transport::connect(endpoint const& ep)
{
    ep.check(traits::value);
    const traits::endpoint_data& tcp_data = ep.get<traits::endpoint_data>();
    resolver_type::query query(tcp_data.host,
            std::to_string(tcp_data.port));
    try {
        resolver_type::iterator iter = resolver_.resolve(query);
        socket_.connect(*iter);
        socket_.set_option( socket_type::keep_alive{ true });
    } catch (std::exception const& e) {
        // FIXME connection_failed class
        ::std::cerr << "TCP connection error " << e.what() << "\n";
        throw errors::connection_failed(e.what());
    }
}

void
tcp_transport::connect_async(endpoint const& ep, asio_config::asio_callback cb)
{
    ep.check(traits::value);
    traits::endpoint_data const& tcp_data = ep.get< traits::endpoint_data >();
    resolver_type::query query( tcp_data.host, std::to_string(tcp_data.port) );
    ::psst::asio::async_resolve(resolver_, query,
            ::std::bind( &tcp_transport::handle_resolve, this,
                    std::placeholders::_1, std::placeholders::_2, cb));
}

void
tcp_transport::close()
{
    if (socket_.is_open()) {
        socket_.cancel();
        socket_.close();
    }
}

void
tcp_transport::handle_resolve(asio_config::error_code const& ec,
        resolver_type::iterator endpoint_iterator,
        asio_config::asio_callback cb)
{
    if (!ec) {
        ::psst::asio::async_connect(socket_, endpoint_iterator,
            std::bind( &tcp_transport::handle_connect, this,
                    std::placeholders::_1, cb));
    } else if (cb) {
        cb(ec);
    }
}

void
tcp_transport::handle_connect(asio_config::error_code const& ec,
        asio_config::asio_callback cb)
{
    if (!ec) {
        socket_.set_option( socket_type::keep_alive{ true });
    }
    if (cb) {
        cb(ec);
    }
}

//----------------------------------------------------------------------------
//    SSL/TCP transport
//----------------------------------------------------------------------------
asio_config::ssl_context
ssl_transport::create_context(detail::ssl_options const& opts)
{
    asio_config::ssl_context ctx{ asio_config::ssl_context::sslv23 };
    ctx.set_options(
        asio_config::ssl_context::default_workarounds |
        asio_config::ssl_context::no_sslv2 |
        asio_config::ssl_context::single_dh_use
    );
    if (!opts.verify_file.empty()) {
        ctx.load_verify_file(opts.verify_file);
    } else {
        ctx.set_default_verify_paths();
    }
    if (!opts.cert_file.empty()) {
        if (opts.key_file.empty()) {
            throw errors::runtime_error("No key file for ssl certificate");
        }
        ctx.use_certificate_chain_file(opts.cert_file);
        ctx.use_private_key_file(opts.key_file, asio_ns::ssl::context::pem);
    }
    return ctx;
}

ssl_transport::ssl_transport(asio_config::io_service_ptr io_svc,
        detail::ssl_options const& opts)
    : ctx_(create_context(opts)), resolver_(*io_svc), socket_(*io_svc, ctx_)
{
    if (opts.require_peer_cert) {
        verify_mode_ = asio_ns::ssl::verify_peer
                | asio_ns::ssl::verify_fail_if_no_peer_cert;
    }
    socket_.set_verify_callback(
        std::bind(&ssl_transport::verify_certificate, this,
            std::placeholders::_1, std::placeholders::_2));
}

bool
ssl_transport::verify_certificate(bool preverified, asio_ns::ssl::verify_context& ctx)
{
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    std::cout << "Verifying " << subject_name << "\n";

    return preverified;
}

void
ssl_transport::connect_async(endpoint const& ep, asio_config::asio_callback cb)
{
    ep.check(traits::value);
    traits::endpoint_data const& ssl_data = ep.get< traits::endpoint_data >();

    socket_.set_verify_mode(verify_mode_);
    asio_config::tcp::resolver::query query( ssl_data.host,
            std::to_string(ssl_data.port) );
    ::psst::asio::async_resolve(resolver_, query,
            ::std::bind( &ssl_transport::handle_resolve, this,
                    std::placeholders::_1, std::placeholders::_2, cb));
}

void
ssl_transport::start(asio_config::asio_callback cb)
{
    socket_.set_verify_mode(verify_mode_);
    ::psst::asio::async_handshake(socket_, asio_ns::ssl::stream_base::server,
        std::bind(&ssl_transport::handle_handshake, this,
            std::placeholders::_1, cb));
}

void
ssl_transport::close()
{
    socket_.lowest_layer().close();
}

void
ssl_transport::handle_resolve(asio_config::error_code const& ec,
        asio_config::tcp::resolver::iterator endpoint_iterator,
        asio_config::asio_callback cb)
{
    if (!ec) {
        ::psst::asio::async_connect(socket_.lowest_layer(), endpoint_iterator,
            std::bind( &ssl_transport::handle_connect, this,
                    std::placeholders::_1, cb));
    } else if (cb) {
        cb(ec);
    }
}

void
ssl_transport::handle_connect(asio_config::error_code const& ec,
        asio_config::asio_callback cb)
{
    if (!ec) {
        ::psst::asio::async_handshake(socket_, asio_ns::ssl::stream_base::client,
            std::bind(&ssl_transport::handle_handshake, this,
                std::placeholders::_1, cb));
    } else if (cb) {
        cb(ec);
    }
}

void
ssl_transport::handle_handshake(asio_config::error_code const& ec,
        asio_config::asio_callback cb)
{
    if (cb) {
        cb(ec);
    }
}

//----------------------------------------------------------------------------
//    UDP transport
//----------------------------------------------------------------------------
udp_transport::udp_transport(asio_config::io_service_ptr io_svc)
    : resolver_(*io_svc), socket_(*io_svc)
{
}

void
udp_transport::connect_async(endpoint const& ep, asio_config::asio_callback cb)
{
    ep.check(traits::value);
    traits::endpoint_data const& udp_data = ep.get< traits::endpoint_data >();

    resolver_type::query query(udp_data.host, std::to_string(udp_data.port));
    resolver_.async_resolve(query,
        std::bind(&udp_transport::handle_resolve, this,
                std::placeholders::_1, std::placeholders::_2, cb));
}

void
udp_transport::handle_resolve(asio_config::error_code const& ec,
        resolver_type::iterator endpoint_iterator,
        asio_config::asio_callback cb)
{
    if (!ec) {
        socket_.async_connect(*endpoint_iterator,
            std::bind(&udp_transport::handle_connect, this,
                std::placeholders::_1, cb));
    } else {
        if (cb) cb(ec);
    }
}

void
udp_transport::handle_connect(asio_config::error_code const& ec,
        asio_config::asio_callback cb)
{
    if (cb) cb(ec);
}

void
udp_transport::close()
{
    socket_.close();
}

//----------------------------------------------------------------------------
//    UDP transport listener
//----------------------------------------------------------------------------
transport_listener< void, transport_type::udp >::transport_listener(asio_config::io_service_ptr svc)
        : udp_transport(svc)
{
}

void
transport_listener< void, transport_type::udp >::open(endpoint const& ep)
{
    endpoint_type proto_ep = traits::create_endpoint(io_service_, ep);
    socket_.open(proto_ep.protocol());
    socket_.set_option(socket_type::reuse_address(true));
    socket_.bind(proto_ep);

    // Start receive
}

endpoint
transport_listener< void, transport_type::udp >::local_endpoint() const
{
    return traits::get_endpoint_data(socket_.local_endpoint());
}

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
//----------------------------------------------------------------------------
//    UNIX socket transport
//----------------------------------------------------------------------------

socket_transport::socket_transport(asio_config::io_service_ptr io_svc)
    : socket_(*io_svc)
{
}

void
socket_transport::connect_async(endpoint const& ep, asio_config::asio_callback cb)
{
    ep.check(traits::value);
    traits::endpoint_data const& socket_data = ep.get< traits::endpoint_data >();

    ::psst::asio::async_connect(socket_, endpoint_type{ socket_data.path },
            std::bind(&socket_transport::handle_connect, this,
                    std::placeholders::_1, cb));
}

void
socket_transport::close()
{
    socket_.close();
}

void
socket_transport::handle_connect(asio_config::error_code const& ec, asio_config::asio_callback cb)
{
    if (cb) cb(ec);
}
#endif /* BOOST_ASIO_HAS_LOCAL_SOCKETS */

}  // namespace core
}  // namespace wire
