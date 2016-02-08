/*
 * tcp_sparring_client.hpp
 *
 *  Created on: Feb 8, 2016
 *      Author: zmij
 */

#ifndef TRANSPORT_TCP_SPARRING_CLIENT_HPP_
#define TRANSPORT_TCP_SPARRING_CLIENT_HPP_

#include <wire/asio_config.hpp>

namespace wire {
namespace test {
namespace tcp {

class client {
public:
	client(asio_config::io_service& svc);
	virtual ~client()
	{
	}
private:
	void
	handle_connect(asio_config::error_code const& ec);
private:
	asio_config::tcp::socket socket_;
};

} /* namespace tcp */
} /* namespace test */
} /* namespace wire */

#endif /* TRANSPORT_TCP_SPARRING_CLIENT_HPP_ */
