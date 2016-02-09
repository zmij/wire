/*
 * tcp_sparring_client.hpp
 *
 *  Created on: Feb 8, 2016
 *      Author: zmij
 */

#ifndef TRANSPORT_SSL_SPARRING_CLIENT_HPP_
#define TRANSPORT_SSL_SPARRING_CLIENT_HPP_

#include <wire/asio_config.hpp>
#include <wire/core/transport.hpp>

namespace wire {
namespace test {
namespace ssl {

class client {
public:
	client(asio_config::io_service_ptr svc);
	virtual ~client()
	{}
private:
	void
	handle_connect(asio_config::error_code const& ec);
//	void
//	handle_handshake(asio_config::error_code const& ec);
private:
	core::ssl_transport transport_;
};

} /* namespace tcp */
} /* namespace test */
} /* namespace wire */

#endif /* TRANSPORT_SSL_SPARRING_CLIENT_HPP_ */
