/*
 * ssl_transport_server_test.cpp
 *
 *  Created on: Feb 9, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/core/transport.hpp>

#include "sparring_test.hpp"
#include "config.hpp"

namespace wire {
namespace core {
namespace test {

struct ssl_session : std::enable_shared_from_this< ssl_session > {
	typedef transport_type_traits< transport_type::ssl >	traits;
	typedef ssl_transport::options							options;

	ssl_session(asio_config::io_service_ptr svc,
			options const& opts, bool& connected)
		: svc_(svc), transport_{svc, opts}, connected_(connected)
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
		std::cerr << "Start SSL session\n";
		transport_.start(
			::std::bind(&ssl_session::handle_connected, shared_from_this(),
				::std::placeholders::_1));
	}

	void
	set_verify_client()
	{
		transport_.set_verify_mode(ASIO_NS::ssl::verify_peer |
				ASIO_NS::ssl::verify_fail_if_no_peer_cert);
	}

	void
	handle_connected(asio_config::error_code const& ec)
	{
		if (!ec) {
			std::cerr << "Connected OK\n";
			connected_ = true;
		} else {
			std::cerr << "Not connected: " << ec.message() << "\n";
			connected_ = false;
		}
		svc_->stop();
	}

	asio_config::io_service_ptr svc_;
	ssl_transport	transport_;
	bool&			connected_;
};

class SSLServer : public wire::test::transport::SparringTest {
public:
	typedef transport_listener< ssl_session, transport_type::ssl >	server_type;
protected:
	SSLServer()
	: SparringTest(),
	  server_(io_svc,
	  [&](asio_config::io_service_ptr svc) {
		return ::std::make_shared< ssl_session >(svc, ssl_options, connected_);
	  })
	{
	}

	void
	SetUp() override
	{
	}

	void
	ReadSparringOutput(std::istream& is) override
	{
	}

	void
	SetupArgs(args_type& args) override
	{
		args.insert(args.end(), {
			"--transport", "ssl",
			"--client",
			"--port", ::std::to_string(port_)
		});
		std::copy(add_args.begin(), add_args.end(), std::back_inserter(args));
	}

	bool							connected_ = false;
	server_type						server_;
	uint16_t						port_ = 0;

	args_type						add_args;
	ssl_session::options			ssl_options;
};

TEST_F(SSLServer, Listen)
{
	ssl_options.verify_file = wire::test::CA_ROOT;
	ssl_options.cert_file = wire::test::SERVER_CERT;
	ssl_options.key_file = wire::test::SERVER_KEY;
	ssl_options.require_peer_cert = false;

	add_args.insert(add_args.end(), {
		"--verify-file", wire::test::CA_ROOT
	});

	endpoint ep = endpoint::ssl("127.0.0.1", 0);
	server_.open(ep);
	server_type::endpoint_type proto_ep = server_.local_endpoint();
	std::cerr << proto_ep.address() << ":" << proto_ep.port() << "\n";
	ASSERT_LT(0, proto_ep.port());
	port_ = proto_ep.port();
	StartPartner();
	io_svc->run();
	EXPECT_TRUE(connected_);
}

TEST_F(SSLServer, VerifyClient)
{
	ssl_options.verify_file = wire::test::CA_ROOT;
	ssl_options.cert_file = wire::test::SERVER_CERT;
	ssl_options.key_file = wire::test::SERVER_KEY;
	ssl_options.require_peer_cert = true;

	add_args.insert(add_args.end(), {
		"--verify-file", wire::test::CA_ROOT,
		"--cert-file", wire::test::CLIENT_CERT,
		"--key-file", wire::test::CLIENT_KEY
	});

	endpoint ep = endpoint::ssl("127.0.0.1", 0);
	server_.open(ep);
	server_type::endpoint_type proto_ep = server_.local_endpoint();
	std::cerr << proto_ep.address() << ":" << proto_ep.port() << "\n";
	ASSERT_LT(0, proto_ep.port());
	port_ = proto_ep.port();
	StartPartner();
	io_svc->run();
	EXPECT_TRUE(connected_);
}


}  // namespace test
}  // namespace core
}  // namespace wire
