/*
 * ssl_transport_test.cpp
 *
 *  Created on: 27 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>
#include <wire/core/transport.hpp>

#include "sparring_test.hpp"
#include "config.hpp"

namespace wire {
namespace core {
namespace test {

class SSL : public wire::test::transport::SparringTest {
protected:
	SSL() : SparringTest(),
		ssl_ctx(std::make_shared<asio_config::ssl_context>(asio_config::ssl_context::sslv23))
	{
	}
	void
	SetUp() override
	{
		ssl_ctx->set_options(
			asio_config::ssl_context::default_workarounds |
			asio_config::ssl_context::no_sslv2 |
			asio_config::ssl_context::single_dh_use
		);
		ssl_ctx->load_verify_file( wire::test::CA_ROOT);
	}
	void
	SetupArgs(args_type& args) override
	{
		args.insert(args.end(), {
			"--transport", "ssl",
			"--connections", "1"
		});
		if (!alternate_cert) {
			args.insert(args.end(), {
				"--cert-file", wire::test::SERVER_CERT,
				"--key-file", wire::test::SERVER_KEY,
			});
		} else {
			args.insert(args.end(), {
				"--cert-file", wire::test::OTHER_CERT,
				"--key-file", wire::test::OTHER_KEY,
			});
		}
		if (verify_peer) {
			args.insert(args.end(), {
				"--verify-file", (alternate_cert ? wire::test::OTHER_CA : wire::test::CA_ROOT),
				"--verify-client"
			});
		}
	}
	void
	ReadSparringOutput(std::istream& is) override
	{
		detail::ssl_endpoint_data ssl_data{ "127.0.0.1", 0 };
		is >> ssl_data.port;
		std::cerr << "Sparring partner is listening to port " << ssl_data.port << "\n";
		endpoint_ = endpoint{ ssl_data };
	}

	endpoint						endpoint_;
	asio_config::ssl_context_ptr	ssl_ctx;
	bool							alternate_cert	= false;
	bool							verify_peer		= false;
};

TEST_F(SSL, Connect)
{
	StartPartner();
	ASSERT_NE(0, child_.pid);
	ASSERT_EQ(transport_type::ssl, endpoint_.transport());
	ASSERT_NE(0, boost::get<detail::ssl_endpoint_data>(endpoint_.data()).port);

	ssl_transport ssl(io_svc, ssl_ctx);
	bool connected = false;
	ssl.connect_async(endpoint_,
	[&]( asio_config::error_code const& ec ) {
		if (!ec)
			connected = true;
		ssl.close();
	});

	io_svc->run();
	EXPECT_TRUE(connected);
}

TEST_F(SSL, InvalidServerCert)
{
	alternate_cert = true;
	StartPartner();
	ASSERT_NE(0, child_.pid);
	ASSERT_EQ(transport_type::ssl, endpoint_.transport());
	ASSERT_NE(0, boost::get<detail::ssl_endpoint_data>(endpoint_.data()).port);

	ssl_transport ssl(io_svc, ssl_ctx);
	bool connected = false;
	ssl.connect_async(endpoint_,
	[&]( asio_config::error_code const& ec ) {
		if (!ec)
			connected = true;
		ssl.close();
	});

	io_svc->run();
	EXPECT_FALSE(connected);
}

TEST_F(SSL, NoClientCert)
{
	verify_peer = true;
	StartPartner();
	ASSERT_NE(0, child_.pid);
	ASSERT_EQ(transport_type::ssl, endpoint_.transport());
	ASSERT_NE(0, boost::get<detail::ssl_endpoint_data>(endpoint_.data()).port);

	ssl_transport ssl(io_svc, ssl_ctx);
	bool connected = false;
	ssl.connect_async(endpoint_,
	[&]( asio_config::error_code const& ec ) {
		if (!ec)
			connected = true;
		ssl.close();
	});

	io_svc->run();
	EXPECT_FALSE(connected);
}

TEST_F(SSL, VerifyClientCert)
{
	verify_peer = true;
	StartPartner();
	ssl_ctx->use_certificate_chain_file(wire::test::CLIENT_CERT);
	ssl_ctx->use_private_key_file(wire::test::CLIENT_KEY, asio_config::ssl_context::pem);
	ASSERT_NE(0, child_.pid);
	ASSERT_EQ(transport_type::ssl, endpoint_.transport());
	ASSERT_NE(0, boost::get<detail::ssl_endpoint_data>(endpoint_.data()).port);

	ssl_transport ssl(io_svc, ssl_ctx);
	bool connected = false;
	ssl.connect_async(endpoint_,
	[&]( asio_config::error_code const& ec ) {
		if (!ec)
			connected = true;
		ssl.close();
	});

	io_svc->run();
	EXPECT_TRUE(connected);
}

TEST_F(SSL, ClientCertInvalidServer)
{
	verify_peer		= true;
	alternate_cert	= true;
	StartPartner();
	ssl_ctx->use_certificate_chain_file(wire::test::CLIENT_CERT);
	ssl_ctx->use_private_key_file(wire::test::CLIENT_KEY, asio_config::ssl_context::pem);
	ASSERT_NE(0, child_.pid);
	ASSERT_EQ(transport_type::ssl, endpoint_.transport());
	ASSERT_NE(0, boost::get<detail::ssl_endpoint_data>(endpoint_.data()).port);

	ssl_transport ssl(io_svc, ssl_ctx);
	bool connected = false;
	ssl.connect_async(endpoint_,
	[&]( asio_config::error_code const& ec ) {
		if (!ec)
			connected = true;
		ssl.close();
	});

	io_svc->run();
	EXPECT_FALSE(connected);
}

TEST_F(SSL, ReadWrite)
{
	verify_peer		= true;
	StartPartner();
	ssl_ctx->use_certificate_chain_file(wire::test::CLIENT_CERT);
	ssl_ctx->use_private_key_file(wire::test::CLIENT_KEY, asio_config::ssl_context::pem);
	ASSERT_NE(0, child_.pid);
	ASSERT_EQ(transport_type::ssl, endpoint_.transport());
	ASSERT_NE(0, boost::get<detail::ssl_endpoint_data>(endpoint_.data()).port);

	ssl_transport ssl(io_svc, ssl_ctx);

	bool connected = false;
	size_t errors = 0;
	const std::string test_str("TestString");
	ASIO_NS::streambuf in_buffer;
	std::string input_str;

	ssl.connect_async(endpoint_,
	[&]( asio_config::error_code const& ec ) {
		if (!ec) {
			connected = true;
			ssl.async_write( ASIO_NS::buffer(test_str),
			[&](asio_config::error_code const& ec, std::size_t bytes_transferred){
				if (!ec) {
					ssl.async_read(in_buffer,
					[&](asio_config::error_code const& ec, std::size_t bytes_transferred) {
						if (!ec) {
							std::istream is(&in_buffer);
							is >> input_str;
						} else {
							errors++;
						}
					});
				} else {
					errors++;
				}
			});
		} else {
			errors++;
		}
	});

	io_svc->run();
	EXPECT_EQ(0, errors);
	EXPECT_EQ(test_str, input_str);
}

} // namespace test
}  // namespace core
}  // namespace wire
