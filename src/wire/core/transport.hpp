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
#include <future>

namespace wire {
namespace core {

struct tcp_transport;
struct ssl_transport;
struct udp_transport;
struct socket_transport;

template < transport_type >
struct transport_type_traits;

template<>
struct transport_type_traits< transport_type::tcp > {
	static const transport_type	value = transport_type::tcp;

	typedef tcp_transport				type;
	typedef detail::tcp_endpoint_data	endpoint_data;

	typedef asio_config::tcp			protocol;
	typedef protocol::socket			socket_type;
	typedef protocol::endpoint			endpoint_type;
	typedef protocol::resolver			resolver_type;
};

template<>
struct transport_type_traits< transport_type::ssl > {
	static const transport_type	value = transport_type::ssl;

	typedef ssl_transport				type;
	typedef detail::ssl_endpoint_data	endpoint_data;

	typedef asio_config::tcp			protocol;
	typedef ASIO_NS::ssl::stream< protocol::socket > socket_type;
	typedef protocol::endpoint			endpoint_type;
	typedef protocol::resolver			resolver_type;
};

template<>
struct transport_type_traits< transport_type::udp > {
	static const transport_type	value = transport_type::udp;

	typedef udp_transport				type;
	typedef detail::udp_endpoint_data	endpoint_data;

	typedef asio_config::udp			protocol;
	typedef protocol::socket			socket_type;
	typedef protocol::endpoint			endpoint_type;
	typedef protocol::resolver			resolver_type;
};

template<>
struct transport_type_traits< transport_type::socket > {
	static const transport_type	value = transport_type::socket;

	typedef socket_transport			type;
	typedef detail::socket_endpoint_data	endpoint_data;

	typedef asio_config::local_socket	protocol;
	typedef protocol::socket			socket_type;
	typedef protocol::endpoint			endpoint_type;
};


struct tcp_transport {
	typedef transport_type_traits< transport_type::tcp >	traits;
	typedef traits::socket_type								socket_type;
	typedef traits::resolver_type							resolver_type;

	tcp_transport(asio_config::io_service_ptr);

	void
	connect(endpoint const& ep);
	void
	connect_async(endpoint const& ep, asio_config::asio_callback);
	void
	close();

	template < typename BufferType, typename HandlerType >
	void
	async_write(BufferType const& buffer, HandlerType const& handler)
	{
		ASIO_NS::async_write(socket_, buffer, handler);
	}

	template < typename BufferType >
	std::future< std::size_t >
	async_write(BufferType const& buffer)
	{
		return ASIO_NS::async_write(socket_, buffer, asio_config::use_future);
	}

	template < typename BufferType, typename HandlerType >
	void
	async_read(BufferType& buffer, HandlerType handler)
	{
		ASIO_NS::async_read(socket_, buffer,
				ASIO_NS::transfer_at_least(1), handler);
	}

	template < typename BufferType >
	std::future< std::size_t >
	async_read(BufferType& buffer)
	{
		return ASIO_NS::async_read(socket_, buffer,
				ASIO_NS::transfer_at_least(1), asio_config::use_future);
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
			resolver_type::iterator endpoint_iterator,
			asio_config::asio_callback);
	void
	handle_connect(asio_config::error_code const& ec, asio_config::asio_callback);
private:
	resolver_type	resolver_;
	socket_type		socket_;

	// TODO timeout settings
};

struct ssl_transport {
	typedef transport_type_traits< transport_type::ssl >	traits;
	typedef traits::socket_type								socket_type;
	typedef traits::resolver_type							resolver_type;

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
			resolver_type::iterator endpoint_iterator,
			asio_config::asio_callback);
	void
	handle_connect(asio_config::error_code const& ec, asio_config::asio_callback);
	void
	handle_handshake(asio_config::error_code const& ec, asio_config::asio_callback);
private:
	resolver_type	resolver_;
	socket_type		socket_;
};

struct udp_transport {
	typedef transport_type_traits< transport_type::udp >	traits;
	typedef traits::protocol								protocol;
	typedef traits::socket_type								socket_type;
	typedef traits::endpoint_type							endpoint_type;
	typedef traits::resolver_type							resolver_type;

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
			resolver_type::iterator endpoint_iterator,
			asio_config::asio_callback);
	void
	handle_connect(asio_config::error_code const&, asio_config::asio_callback);
private:
	resolver_type	resolver_;
	socket_type		socket_;
};

struct socket_transport {
	typedef transport_type_traits< transport_type::socket >	traits;
	typedef traits::socket_type			socket_type;
	typedef traits::endpoint_type		endpoint_type;

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
