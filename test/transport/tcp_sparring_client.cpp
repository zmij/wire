/*
 * tcp_sparring_client.cpp
 *
 *  Created on: Feb 8, 2016
 *      Author: zmij
 */

#include "tcp_sparring_client.hpp"
#include "sparring_options.hpp"
#include <iostream>

namespace wire {
namespace test {
namespace tcp {

client::client(asio_config::io_service_ptr svc)
	: transport_(svc)
{
	transport_.connect_async(
		core::endpoint::tcp("127.0.0.1", sparring_options::instance().port),
		::std::bind(&client::handle_connect, this,
							std::placeholders::_1));
}

void
client::handle_connect(asio_config::error_code const& ec)
{
	std::cerr << "[SPARRING] Connected to test tcp server\n";
}

} /* namespace tcp */
} /* namespace test */
} /* namespace wire */
