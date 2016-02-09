/*
 * tcp_sparring.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#ifndef TRANSPORT_SOCKET_SPARRING_HPP_
#define TRANSPORT_SOCKET_SPARRING_HPP_

#include <wire/asio_config.hpp>

namespace wire {
namespace test {
namespace socket {

class session {
public:
	typedef asio_config::local_socket::socket socket_type;
	enum {
		max_length = 1024
	};
public:
	session(asio_config::io_service& svc, std::size_t requests)
		: socket_(svc), requests_(requests), limit_requests_(requests_ > 0)
	{
	}
	~session()
	{
		socket_.close();
	}

	socket_type&
	socket()
	{
		return socket_;
	}

	void
	start();
private:
	void
	handle_read(asio_config::error_code const& ec, size_t bytes_transferred);
	void
	handle_write(asio_config::error_code const& ec, size_t bytes_transferred);
private:
	socket_type	socket_;
	char data_[ max_length ];
	std::size_t	requests_;
	bool limit_requests_;
};

class server {
public:
	typedef asio_config::local_socket	protocol;
	typedef protocol::endpoint			endpoint_type;
	typedef protocol::acceptor			acceptor;
public:
	server(asio_config::io_service_ptr svc);
	~server();
private:
	void
	start_accept();
	void
	handle_accept(session*, asio_config::error_code const&);

	void
	handle_stop();
private:
	asio_config::io_service_ptr	io_service_;
	acceptor					acceptor_;
	ASIO_NS::signal_set			signals_;

	std::size_t					connections_;
	bool						limit_connections_;
	std::size_t					requests_;
};

}  // namespace socket
}  // namespace test
}  // namespace wire

#endif /* TRANSPORT_SOCKET_SPARRING_HPP_ */
