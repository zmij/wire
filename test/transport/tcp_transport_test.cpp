/*
 * tcp_transport_test.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/core/transport.hpp>
#include <boost/process.hpp>
#include <boost/process/mitigate.hpp>
#include <boost/iostreams/stream.hpp>

#include "sparring_test.hpp"

namespace wire {
namespace core {
namespace test {

namespace bp = boost::process;
namespace bpi = boost::process::initializers;
namespace bio = boost::iostreams;

class TCP : public wire::test::transport::SparringTest {
protected:
	void
	SetupArgs(args_type& args)
	{
		args.insert(args.end(), {
			"--transport", "tcp",
			"--connections", "1"
		});
	}
	void
	ReadSparringOutput(std::istream& is)
	{
		detail::tcp_endpoint_data tcp_data{ "127.0.0.1", 0 };
		is >> tcp_data.port;
		std::cerr << "Sparring partner is listening to port " << tcp_data.port << "\n";
		endpoint_ = endpoint{ tcp_data };
	}
	endpoint		endpoint_;
};

TEST_F(TCP, Connect)
{
	ASSERT_NE(0, child_.pid);
	ASSERT_EQ(transport_type::tcp, endpoint_.transport());
	ASSERT_NE(0, boost::get<detail::tcp_endpoint_data>(endpoint_.data()).port);
	asio_config::io_service_ptr io_svc =
			std::make_shared< asio_config::io_service >();
	tcp_transport tcp(io_svc);
	bool connected = false;
	tcp.connect_async(endpoint_,
	[&]( asio_config::error_code const& ec ) {
		if (!ec)
			connected = true;
		tcp.close();
	});

	io_svc->run();
	EXPECT_TRUE(connected);
}

TEST_F(TCP, ConnectFail)
{
	ASSERT_NE(0, child_.pid);
	ASSERT_EQ(transport_type::tcp, endpoint_.transport());
	ASSERT_NE(0, boost::get<detail::tcp_endpoint_data>(endpoint_.data()).port);
	StopPartner();
	asio_config::io_service_ptr io_svc =
			std::make_shared< asio_config::io_service >();
	tcp_transport tcp(io_svc);
	bool connected = false;
	tcp.connect_async(endpoint_,
	[&]( asio_config::error_code const& ec ) {
		if (!ec)
			connected = true;
		tcp.close();
	});

	io_svc->run();
	EXPECT_FALSE(connected);
}

TEST_F(TCP, ReadWrite)
{
	ASSERT_NE(0, child_.pid);
	ASSERT_EQ(transport_type::tcp, endpoint_.transport());
	ASSERT_NE(0, boost::get<detail::tcp_endpoint_data>(endpoint_.data()).port);

	asio_config::io_service_ptr io_svc =
			std::make_shared< asio_config::io_service >();
	tcp_transport tcp(io_svc);

	bool connected = false;
	size_t errors = 0;
	const std::string test_str("TestString");
	ASIO_NS::streambuf in_buffer;
	std::string input_str;

	tcp.connect_async(endpoint_,
	[&]( asio_config::error_code const& ec ) {
		if (!ec) {
			connected = true;
			tcp.async_write( ASIO_NS::buffer(test_str),
			[&](asio_config::error_code const& ec, std::size_t bytes_transferred){
				if (!ec) {
					tcp.async_read(in_buffer,
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

}  // namespace test
}  // namespace core
}  // namespace wire
