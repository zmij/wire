/*
 * tcp_sparring_client.hpp
 *
 *  Created on: Feb 8, 2016
 *      Author: zmij
 */

#ifndef TRANSPORT_TCP_SPARRING_CLIENT_HPP_
#define TRANSPORT_TCP_SPARRING_CLIENT_HPP_

#include <wire/asio_config.hpp>
#include <wire/core/transport.hpp>

namespace wire {
namespace test {
namespace tcp {

class client {
public:
	client(asio_config::io_service_ptr svc);
	virtual ~client()
	{
	}
private:
	void
	handle_connect(asio_config::error_code const& ec);
private:
	core::tcp_transport transport_;
};

} /* namespace ssl */
} /* namespace test */
} /* namespace wire */

#endif /* TRANSPORT_TCP_SPARRING_CLIENT_HPP_ */
