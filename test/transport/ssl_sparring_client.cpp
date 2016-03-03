/*
 * tcp_sparring_client.cpp
 *
 *  Created on: Feb 8, 2016
 *      Author: zmij
 */

#include "ssl_sparring_client.hpp"
#include "sparring_options.hpp"
#include <iostream>

namespace wire {
namespace test {
namespace ssl {

client::client(asio_config::io_service_ptr svc)
	: transport_(svc,
		core::detail::ssl_options{
			sparring_options::instance().verify_file,
			sparring_options::instance().cert_file,
			sparring_options::instance().key_file,
			true
		})
{
	sparring_options& opts = sparring_options::instance();
	transport_.connect_async(
		core::endpoint::ssl("127.0.0.1", sparring_options::instance().port),
		::std::bind(&client::handle_connect, this,
							std::placeholders::_1));
}

void
client::handle_connect(asio_config::error_code const& ec)
{
	if (!ec) {
		std::cerr << "[SPARRING] Connected to test ssl server\n";
	} else {
		std::cerr << "[SPARRING] Failed to connect to test ssl server: "
				<< ec.message() << "\n";
	}
}

} /* namespace ssl */
} /* namespace test */
} /* namespace wire */
