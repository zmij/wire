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
#include <wire/core/detail/configuration_options.hpp>

#include <memory>
#include <future>

namespace wire {
namespace core {

#ifdef SO_REUSEPORT
typedef ASIO_NS::detail::socket_option::boolean<
		BOOST_ASIO_OS_DEF(SOL_SOCKET), SO_REUSEPORT > reuse_port;
#endif

struct tcp_transport;
struct ssl_transport;
struct udp_transport;
struct socket_transport;

template < transport_type >
struct transport_type_traits;

template<>
struct transport_type_traits< transport_type::tcp > {
	static constexpr transport_type	value = transport_type::tcp;
	static constexpr bool stream_oriented = true;

	typedef tcp_transport				type;
	typedef detail::tcp_endpoint_data	endpoint_data;

	typedef asio_config::tcp			protocol;
	typedef protocol::socket			socket_type;
	typedef protocol::socket			listen_socket_type;
	typedef protocol::endpoint			endpoint_type;
	typedef protocol::resolver			resolver_type;
	typedef protocol::acceptor			acceptor_type;

	static endpoint_type
	create_endpoint(asio_config::io_service_ptr svc, endpoint const&);
};

template<>
struct transport_type_traits< transport_type::ssl > {
	static constexpr transport_type	value = transport_type::ssl;
	static constexpr bool stream_oriented = true;

	typedef ssl_transport				type;
	typedef detail::ssl_endpoint_data	endpoint_data;

	typedef asio_config::tcp			protocol;
	typedef ASIO_NS::ssl::stream< protocol::socket > socket_type;
	typedef socket_type::lowest_layer_type	listen_socket_type;
	typedef protocol::endpoint			endpoint_type;
	typedef protocol::resolver			resolver_type;
	typedef protocol::acceptor			acceptor_type;

	static endpoint_type
	create_endpoint(asio_config::io_service_ptr svc, endpoint const&);
};

template<>
struct transport_type_traits< transport_type::udp > {
	static constexpr transport_type	value = transport_type::udp;
	static constexpr bool stream_oriented = false;

	typedef udp_transport				type;
	typedef detail::udp_endpoint_data	endpoint_data;

	typedef asio_config::udp			protocol;
	typedef protocol::socket			socket_type;
	typedef protocol::socket			listen_socket_type;
	typedef protocol::endpoint			endpoint_type;
	typedef protocol::resolver			resolver_type;

	static endpoint_type
	create_endpoint(asio_config::io_service_ptr svc, endpoint const&);
};

template<>
struct transport_type_traits< transport_type::socket > {
	static constexpr transport_type	value = transport_type::socket;
	static constexpr bool stream_oriented = true;

	typedef socket_transport			type;
	typedef detail::socket_endpoint_data	endpoint_data;

	typedef asio_config::local_socket	protocol;
	typedef protocol::socket			socket_type;
	typedef protocol::socket			listen_socket_type;
	typedef protocol::endpoint			endpoint_type;
	typedef protocol::acceptor			acceptor_type;

	static endpoint_type
	create_endpoint(asio_config::io_service_ptr svc, endpoint const&);
};


struct tcp_transport {
	typedef transport_type_traits< transport_type::tcp >	traits;
	typedef traits::socket_type								socket_type;
	typedef traits::listen_socket_type						listen_socket_type;
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
	async_read(BufferType&& buffer, HandlerType handler)
	{
		ASIO_NS::async_read(socket_, std::forward< BufferType&& >(buffer),
				ASIO_NS::transfer_at_least(1), handler);
	}

	template < typename BufferType >
	std::future< std::size_t >
	async_read(BufferType&& buffer)
	{
		return ASIO_NS::async_read(socket_, std::forward< BufferType&& >(buffer),
				ASIO_NS::transfer_at_least(1), asio_config::use_future);
	}

	listen_socket_type&
	socket()
	{ return socket_; }
	listen_socket_type const&
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
	tcp_transport(tcp_transport const&) = delete;
	tcp_transport&
	operator = (tcp_transport const&) = delete;
private:
	resolver_type	resolver_;
	socket_type		socket_;

	// TODO timeout settings
};

struct ssl_transport {
	typedef transport_type_traits< transport_type::ssl >	traits;
	typedef traits::socket_type								socket_type;
	typedef traits::listen_socket_type						listen_socket_type;
	typedef traits::resolver_type							resolver_type;
	typedef ASIO_NS::ssl::verify_mode						verify_mode;

	static asio_config::ssl_context
	create_context(detail::ssl_options const&);

	ssl_transport(asio_config::io_service_ptr, detail::ssl_options const&
			= detail::ssl_options {});

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
	set_verify_mode(verify_mode mode)
	{ verify_mode_ = mode; }

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

	listen_socket_type&
	socket()
	{ return socket_.lowest_layer(); }
	listen_socket_type const&
	socket() const
	{ return socket_.lowest_layer(); }
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
	ssl_transport(ssl_transport const&) = delete;
	ssl_transport&
	operator = (ssl_transport const&) = delete;
private:
	asio_config::ssl_context	ctx_;
	resolver_type				resolver_;
	socket_type					socket_;
	verify_mode					verify_mode_ = ASIO_NS::ssl::verify_peer;
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
	udp_transport(udp_transport const&) = delete;
	udp_transport&
	operator = (udp_transport const&) = delete;
protected:
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
	socket_transport(socket_transport const&) = delete;
	socket_transport&
	operator = (socket_transport const&) = delete;
private:
	socket_type						socket_;
};

template< typename Session, transport_type Type >
struct transport_listener {
	typedef transport_type_traits< Type >	traits;
	typedef typename traits::acceptor_type	acceptor_type;
	typedef typename traits::endpoint_type	endpoint_type;
	typedef typename traits::endpoint_data	endpoint_data;
	typedef Session							session_type;
	typedef std::shared_ptr<Session>		session_ptr;
	typedef std::function< session_ptr(asio_config::io_service_ptr) >
											session_factory;

	transport_listener(asio_config::io_service_ptr, session_factory);

	void
	open(endpoint const&, bool reuse_port = false);

	endpoint_type
	local_endpoint() const;
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
	operator = (transport_listener const&) = delete;
private:
	asio_config::io_service_ptr	io_service_;
	acceptor_type				acceptor_;
	session_factory				factory_;
};

template <>
struct transport_listener< void, transport_type::udp > : udp_transport {
	typedef transport_type_traits< transport_type::udp >	traits;
	typedef traits::endpoint_type							endpoint_type;
	typedef traits::endpoint_data							endpoint_data;

	transport_listener(asio_config::io_service_ptr);

	void
	open(endpoint const&);

	endpoint_type
	local_endpoint() const;
private:
	transport_listener(transport_listener const&) = delete;
	transport_listener&
	operator = (transport_listener const&) = delete;
private:
	asio_config::io_service_ptr	io_service_;
};

}  // namespace core
}  // namespace wire

#include <wire/core/transport.inl>

#endif /* WIRE_CORE_TRANSPORT_HPP_ */
