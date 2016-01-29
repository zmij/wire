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
//	TCP transport
//----------------------------------------------------------------------------
tcp_transport::tcp_transport(asio_config::io_service_ptr io_svc)
	: resolver_(*io_svc), socket_(*io_svc)
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
	} catch (std::exception const& e) {
		// FIXME connection_failed class
		throw errors::connection_failed(e.what());
	}
}

void
tcp_transport::connect_async(endpoint const& ep, asio_config::asio_callback cb)
{
	ep.check(traits::value);
	traits::endpoint_data const& tcp_data = ep.get< traits::endpoint_data >();
	resolver_type::query query( tcp_data.host, std::to_string(tcp_data.port) );
	resolver_.async_resolve(query,
			std::bind( &tcp_transport::handle_resolve, this,
					std::placeholders::_1, std::placeholders::_2, cb));
}

void
tcp_transport::close()
{
	if (socket_.is_open())
		socket_.close();
}

void
tcp_transport::handle_resolve(asio_config::error_code const& ec,
		resolver_type::iterator endpoint_iterator,
		asio_config::asio_callback cb)
{
	if (!ec) {
		ASIO_NS::async_connect(socket_, endpoint_iterator,
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
	if (cb) {
		cb(ec);
	}
}

//----------------------------------------------------------------------------
//	SSL/TCP transport
//----------------------------------------------------------------------------
asio_config::ssl_context&
from_ptr_or_default(asio_config::ssl_context_ptr ctx)
{
	// FIXME Check for null pointer
	return *ctx;
}

ssl_transport::ssl_transport(asio_config::io_service_ptr io_svc,
		asio_config::ssl_context_ptr ctx)
	: resolver_(*io_svc), socket_(*io_svc, from_ptr_or_default(ctx))
{
	socket_.set_verify_callback(
		std::bind(&ssl_transport::verify_certificate, this,
			std::placeholders::_1, std::placeholders::_2));
}

bool
ssl_transport::verify_certificate(bool preverified, ASIO_NS::ssl::verify_context& ctx)
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

	socket_.set_verify_mode(ASIO_NS::ssl::verify_peer);
	asio_config::tcp::resolver::query query( ssl_data.host,
			std::to_string(ssl_data.port) );
	resolver_.async_resolve(query,
			std::bind( &ssl_transport::handle_resolve, this,
					std::placeholders::_1, std::placeholders::_2, cb));
}

void
ssl_transport::start(asio_config::asio_callback cb)
{
	socket_.async_handshake(
		ASIO_NS::ssl::stream_base::server,
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
		ASIO_NS::async_connect(socket_.lowest_layer(), endpoint_iterator,
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
		socket_.async_handshake(ASIO_NS::ssl::stream_base::client,
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
//	UDP transport
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
//	UNIX socket transport
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

	socket_.async_connect( endpoint_type{ socket_data.path },
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

}  // namespace core
}  // namespace wire
