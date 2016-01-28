/*
 * transport.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_TRANSPORT_HPP_
#define WIRE_CORE_TRANSPORT_HPP_

#include <wire/asio_config.hpp>
#include <wire/core/endpoint.hpp>
#include <memory>

namespace wire {
namespace core {

struct tcp_transport {
	typedef asio_config::tcp::socket	socket_type;

	tcp_transport(asio_config::io_service_ptr);

	void
	connect_async(endpoint const& ep, asio_config::asio_callback);
	void
	close();

	template < typename BufferType, typename HandlerType >
	void
	async_write(BufferType const& buffer, HandlerType handler)
	{
		ASIO_NS::async_write(socket_, buffer, handler);
	}

	template < typename BufferType, typename HandlerType >
	void
	async_read(BufferType& buffer, HandlerType handler)
	{
		ASIO_NS::async_read(socket_, buffer,
				ASIO_NS::transfer_at_least(1), handler);
	}

	socket_type&
	socket()
	{ return socket_; }
	socket_type const&
	socket() const
	{ return socket_; }
private:
	void
	handle_resolve(asio_config::error_code const& ec,
			asio_config::tcp::resolver::iterator endpoint_iterator,
			asio_config::asio_callback);
	void
	handle_connect(asio_config::error_code const& ec, asio_config::asio_callback);
private:
	asio_config::tcp::resolver		resolver_;
	socket_type						socket_;

	// TODO timeout settings
};

struct ssl_transport {
	typedef asio_config::tcp::socket			tcp_socket;
	typedef ASIO_NS::ssl::stream<tcp_socket>	socket_type;

	ssl_transport(asio_config::io_service_ptr, asio_config::ssl_context_ptr);

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

	void
	close();

	template < typename BufferType, typename HandlerType >
	void
	async_write(BufferType const& buffer, HandlerType handler)
	{
		ASIO_NS::async_write(socket_, buffer, handler);
	}

	template < typename BufferType, typename HandlerType >
	void
	async_read(BufferType& buffer, HandlerType handler)
	{
		ASIO_NS::async_read(socket_, buffer,
				ASIO_NS::transfer_at_least(1), handler);
	}
private:
	bool
	verify_certificate(bool preverified, ASIO_NS::ssl::verify_context& ctx);

	void
	handle_resolve(asio_config::error_code const& ec,
			asio_config::tcp::resolver::iterator endpoint_iterator,
			asio_config::asio_callback);
	void
	handle_connect(asio_config::error_code const& ec, asio_config::asio_callback);
	void
	handle_handshake(asio_config::error_code const& ec, asio_config::asio_callback);
private:
	asio_config::tcp::resolver		resolver_;
	socket_type						socket_;
};

struct udp_transport {
	typedef asio_config::udp	protocol;
	typedef protocol::socket	socket_type;
	typedef protocol::endpoint	endpoint_type;

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

	template < typename BufferType, typename HandlerType >
	void
	async_write(BufferType const& buffer, HandlerType handler)
	{
		socket_.async_send(buffer, handler);
	}
	template < typename BufferType, typename HandlerType >
	void
	async_read(BufferType& buffer, HandlerType handler)
	{
		socket_.async_receive(buffer, handler);
	}
private:
	void
	handle_resolve(asio_config::error_code const& ec,
			asio_config::udp::resolver::iterator endpoint_iterator,
			asio_config::asio_callback);
	void
	handle_connect(asio_config::error_code const&, asio_config::asio_callback);
private:
	asio_config::udp::resolver		resolver_;
	socket_type						socket_;
};

struct socket_transport {
	typedef asio_config::local_socket	protocol;
	typedef protocol::socket			socket_type;
	typedef protocol::endpoint			endpoint_type;

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

	template < typename BufferType, typename HandlerType >
	void
	async_write(BufferType const& buffer, HandlerType handler)
	{
		ASIO_NS::async_write(socket_, buffer, handler);
	}
	template < typename BufferType, typename HandlerType >
	void
	async_read(BufferType& buffer, HandlerType handler)
	{
		ASIO_NS::async_read(socket_, buffer,
				ASIO_NS::transfer_at_least(1), handler);
	}
private:
	void
	handle_connect(asio_config::error_code const&, asio_config::asio_callback);
private:
	socket_type						socket_;
};

}  // namespace core
}  // namespace wire



#endif /* WIRE_CORE_TRANSPORT_HPP_ */
