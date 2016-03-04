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

	tcp_session(asio_config::io_service_ptr svc, bool& connected)
		: svc_(svc), transport_(svc), connected_(connected)
	{
	}

	traits::listen_socket_type&
	socket()
	{
		return transport_.socket();
	}

	void
	start()
	{
		std::cerr << "Start TCP session\n";
		connected_ = true;
		svc_->stop();
	}

	asio_config::io_service_ptr svc_;
	tcp_transport				transport_;
	bool&						connected_;
};

class TCPServer : public wire::test::transport::SparringTest {
public:
	typedef transport_listener< tcp_session, transport_type::tcp >	server_type;
public:
	TCPServer()
		: SparringTest(),
		  server_(io_svc,
		  [&](asio_config::io_service_ptr svc) {
			return ::std::make_shared< tcp_session >(svc, connected_);
		  })
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

	bool		connected_ = false;
	server_type server_;
	uint16_t	port_ = 0;
};

TEST_F(TCPServer, Listen)
{
	endpoint ep = endpoint::tcp("127.0.0.1", 0);
	server_.open(ep);
	endpoint proto_ep = server_.local_endpoint();
	server_type::endpoint_data data = proto_ep.get<server_type::endpoint_data>();
	std::cerr << data.host << ":" << data.port << "\n";
	ASSERT_LT(0, data.port);
	port_ = data.port;
	StartPartner();
	io_svc->run();
	EXPECT_TRUE(connected_);
}

}  // namespace test
}  // namespace core
}  // namespace wire
