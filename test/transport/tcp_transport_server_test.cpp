/*
 * tcp_transport_server_test.cpp
 *
 *  Created on: Feb 8, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/core/transport.hpp>
#include "sparring_test.hpp"

namespace wire {
namespace core {
namespace test {

struct tcp_session : ::std::enable_shared_from_this< tcp_session > {
	typedef transport_type_traits< transport_type::tcp > traits;

	tcp_session(asio_config::io_service_ptr svc)
		: svc_(svc), socket_(*svc)
	{
	}

	traits::socket_type&
	socket()
	{
		return socket_;
	}

	void
	start()
	{
		std::cerr << "Start TCP session\n";
		svc_->stop();
	}

	asio_config::io_service_ptr svc_;
	traits::socket_type	socket_;
};

class TCPServer : public wire::test::transport::SparringTest {
public:
	typedef transport_listener< tcp_session, transport_type::tcp >	server_type;
public:
	TCPServer()
		: SparringTest(), server_(io_svc), port_(0)
	{
	}
protected:
	void
	SetupArgs(args_type& args) override
	{
		args.insert(args.end(), {
			"--transport", "tcp",
			"--client",
			"--port", ::std::to_string(port_)
		});
	}

	void
	ReadSparringOutput(std::istream& is) override
	{
	}

	server_type server_;
	uint16_t	port_;
};

TEST_F(TCPServer, Listen)
{
	endpoint ep = endpoint::tcp("127.0.0.1", 0);
	server_.open(ep);
	server_type::endpoint_type proto_ep = server_.local_endpoint();
	std::cerr << proto_ep.address() << ":" << proto_ep.port() << "\n";
	ASSERT_LT(0, proto_ep.port());
	port_ = proto_ep.port();
	StartPartner();
	io_svc->run();
}

}  // namespace test
}  // namespace core
}  // namespace wire
