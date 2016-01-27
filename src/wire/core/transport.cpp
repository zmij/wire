/*
 * transport.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#include <wire/core/transport.hpp>
#include <wire/errors/exceptions.hpp>

#include <sstream>

namespace wire {
namespace core {

tcp_transport::tcp_transport(asio_config::io_service_ptr io_svc)
	: resolver_(*io_svc), socket_(*io_svc)
{
}

void
tcp_transport::connect_async(endpoint const& ep, asio_config::asio_callback cb)
{
	// Check endpoint data
	if (ep.transport() != transport_type::tcp) {
		throw errors::logic_error(
				"Invalid endpoint transport type ",
				ep.transport(),
				" for tcp_transport" );
	}
	detail::tcp_endpoint_data const& tcp_data =
			boost::get< detail::tcp_endpoint_data >(ep.data());
	if (tcp_data.host.empty()) {
		throw errors::logic_error("Empty host in an endpoint for tcp transport");
	}
	if (tcp_data.port == 0) {
		throw errors::logic_error("Port is not set in an endpoint for tcp transport");
	}

	asio_config::tcp::resolver::query query( tcp_data.host,
			std::to_string(tcp_data.port) );
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
		asio_config::tcp::resolver::iterator endpoint_iterator,
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


}  // namespace core
}  // namespace wire
