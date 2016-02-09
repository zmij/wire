/*
 * tcp_sparring.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#ifndef TRANSPORT_SSL_SPARRING_HPP_
#define TRANSPORT_SSL_SPARRING_HPP_

#include <wire/asio_config.hpp>

namespace wire {
namespace test {
namespace ssl {

class session {
public:
	typedef ASIO_NS::ssl::context				ssl_context;
	typedef asio_config::tcp::socket			socket_type;
	typedef ASIO_NS::ssl::stream< socket_type >	ssl_socket_type;
	enum {
		max_length = 1024
	};
public:
	session(asio_config::io_service& svc, ssl_context& ctx);

	ssl_socket_type::lowest_layer_type&
	socket()
	{
		return socket_.lowest_layer();
	}

	void
	start();
private:
	void
	start_read();
	bool
	verify_certificate(bool preverified, ASIO_NS::ssl::verify_context& ctx);
	void
	handle_handshake(asio_config::error_code const& ec);
	void
	handle_read(asio_config::error_code const& ec, size_t bytes_transferred);
	void
	handle_write(asio_config::error_code const& ec, size_t bytes_transferred);
private:
	ssl_socket_type	socket_;
	char data_[ max_length ];
	std::size_t	requests_;
	bool limit_requests_;
};

class server {
public:
	server(asio_config::io_service_ptr svc);
private:
	void
	start_accept();
	void
	handle_accept(session*, asio_config::error_code const&);
private:
	asio_config::io_service_ptr	io_service_;
	asio_config::tcp::acceptor	acceptor_;
	ASIO_NS::ssl::context		context_;
	std::size_t					connections_;
	bool						limit_connections_;
	std::size_t					requests_;
};

}  // namespace ssl
}  // namespace test
}  // namespace wire

#endif /* TRANSPORT_SSL_SPARRING_HPP_ */
