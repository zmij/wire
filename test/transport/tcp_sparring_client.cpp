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

client::client(asio_config::io_service& svc)
	: socket_(svc)
{
	asio_config::tcp::resolver resolver(svc);
	asio_config::tcp::resolver::query query("127.0.0.1",
			::std::to_string(sparring_options::instance().port));
	auto ep_iter = resolver.resolve(query);
	ASIO_NS::async_connect(socket_, ep_iter,
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
