/*
 * udp_transport_test.cpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/core/transport.hpp>

#include "sparring_test.hpp"

namespace wire {
namespace core {
namespace test {

class UDP : public wire::test::transport::SparringTest {
protected:
	void
	SetUp() override
	{
		StartPartner();
	}

	void
	SetupArgs(args_type& args) override
	{
		args.insert(args.end(), {
			"--transport", "udp",
			"--requests", std::to_string(num_requests)
		});
	}
	void
	ReadSparringOutput(std::istream& is) override
	{
		endpoint_ = endpoint{ ReadEnpointPort< detail::udp_endpoint_data >(is) };
	}

	endpoint	endpoint_;
	std::size_t num_requests = 1;
};

TEST_F(UDP, Connect)
{
	ASSERT_NE(0, child_.pid);
	ASSERT_EQ(transport_type::udp, endpoint_.transport());
	ASSERT_NE(0, boost::get<detail::udp_endpoint_data>(endpoint_.data()).port);

	udp_transport udp(io_svc);
	bool connected = false;
	udp.connect_async(endpoint_,
	[&](asio_config::error_code const& ec) {
		if (!ec)
			connected = true;
		udp.close();
	});

	io_svc->run();
	EXPECT_TRUE(connected);
}

TEST_F(UDP, ReadWrite)
{
	ASSERT_NE(0, child_.pid);
	ASSERT_EQ(transport_type::udp, endpoint_.transport());
	ASSERT_NE(0, boost::get<detail::udp_endpoint_data>(endpoint_.data()).port);

	udp_transport udp(io_svc);

	bool connected = false;
	size_t errors = 0;
	const std::string test_str("TestString");
	std::string input_str;
	char data[256];
	auto in_buffer = ASIO_NS::buffer(data);

	udp.connect_async(endpoint_,
	[&]( asio_config::error_code const& ec ) {
		if (!ec) {
			connected = true;
			udp.async_write( ASIO_NS::buffer(test_str),
			[&](asio_config::error_code const& ec, std::size_t bytes_transferred){
				if (!ec) {
					udp.async_read(in_buffer,
					[&](asio_config::error_code const& ec, std::size_t bytes_transferred) {
						if (!ec) {
							input_str = std::string(data, bytes_transferred);
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
