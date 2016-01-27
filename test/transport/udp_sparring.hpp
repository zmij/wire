/*
 * udp_sparring.hpp
 *
 *  Created on: 28 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef TRANSPORT_UDP_SPARRING_HPP_
#define TRANSPORT_UDP_SPARRING_HPP_

#include <wire/asio_config.hpp>

namespace wire {
namespace test {
namespace udp {

class server {
public:
	enum {
		max_length = 1024
	};
	typedef std::shared_ptr<asio_config::udp::endpoint> udp_endpoint_ptr;
public:
	server(asio_config::io_service& svc);
private:
	void
	start_receive();
	void
	handle_receive(asio_config::error_code const&, std::size_t, udp_endpoint_ptr);
	void
	handle_send(asio_config::error_code const&, std::size_t, udp_endpoint_ptr);
private:
	asio_config::io_service&	io_service_;
	asio_config::udp::socket	socket_;
	char data_[ max_length ];

	std::size_t					requests_;
	bool						limit_requests_;
};

} /* namespace udp */
} /* namespace test */
} /* namespace wire */

#endif /* TRANSPORT_UDP_SPARRING_HPP_ */
