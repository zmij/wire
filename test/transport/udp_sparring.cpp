/*
 * udp_sparring.cpp
 *
 *  Created on: 28 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#include "udp_sparring.hpp"
#include "sparring_options.hpp"
#include <iostream>

namespace wire {
namespace test {
namespace udp {

server::server(asio_config::io_service& svc)
	: io_service_(svc),
	  socket_{ io_service_, asio_config::udp::endpoint{ asio_config::udp::v4(), 0 } },
	  requests_(sparring_options::instance().requests),
	  limit_requests_(requests_ > 0)
{
	asio_config::udp::endpoint ep = socket_.local_endpoint();
	std::cout << ep.port() << std::endl;
	start_receive();
}

void
server::start_receive()
{
	if (!limit_requests_ || requests_ > 0) {
		udp_endpoint_ptr sender_endpoint
			= std::make_shared<asio_config::udp::endpoint>();
		socket_.async_receive_from(ASIO_NS::buffer(data_, max_length), *sender_endpoint,
				std::bind(&server::handle_receive, this,
					std::placeholders::_1, std::placeholders::_2, sender_endpoint));
	}
}

void
server::handle_receive(asio_config::error_code const& ec, std::size_t bytes_transferred,
		udp_endpoint_ptr sender)
{
	if (!ec && bytes_transferred > 0) {
		if (requests_ > 0)
			--requests_;
		socket_.async_send_to(ASIO_NS::buffer(data_, bytes_transferred), *sender,
				std::bind(&server::handle_send, this,
					std::placeholders::_1, std::placeholders::_2, sender));
	} else {
		start_receive();
	}
}

void
server::handle_send(asio_config::error_code const& ec, std::size_t bytes_transferred,
		udp_endpoint_ptr sender)
{
	start_receive();
}

} /* namespace udp */
} /* namespace test */
} /* namespace wire */
