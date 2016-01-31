/*
 * client_connection_test.cpp
 *
 *  Created on: Jan 29, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/core/connection.hpp>
#include <wire/core/transport.hpp>

#include "sparring_test.hpp"
#include "config.hpp"

#include <sstream>

namespace wire {
namespace core {
namespace test {

class Client : public wire::test::transport::SparringTest {

protected:
	void
	SetupArgs(args_type& args) override
	{
		std::ostringstream os;
		os << current_transport;
		args.insert(args.end(), {
			"--transport", os.str(),
		});

		if (!add_args.empty()) {
			args.insert(args.end(), add_args.begin(), add_args.end());
		}
	}
	void
	ReadSparringOutput(std::istream& is) override
	{
		switch (current_transport) {
			case transport_type::tcp:
				endpoint_ = endpoint{ ReadEnpointPort< detail::tcp_endpoint_data >(is) };
				break;
			case transport_type::ssl:
				endpoint_ = endpoint{ ReadEnpointPort< detail::ssl_endpoint_data >(is) };
				break;
			default:
				break;
		}
	}

	endpoint		endpoint_;
	transport_type 	current_transport;
	args_type		add_args;
};

TEST_F(Client, TCPConnect)
{
	typedef transport_type_traits< transport_type::tcp > used_transport;
	current_transport = used_transport::value;
	add_args.insert(add_args.end(), {
		"--validate-message",
		"-r2"
	});
	StartPartner();

	ASSERT_NE(0, child_.pid);
	ASSERT_EQ(used_transport::value, endpoint_.transport());
	ASSERT_NE(0, endpoint_.get< used_transport::endpoint_data >().port);

	connection c(io_svc);

	bool connected = false;
	bool error = false;

	c.connect_async(endpoint_,
	[&](){
		connected = true;
		c.close();
	},
	[&](std::exception_ptr) {
		error = true;
	});

	io_svc->run();

	EXPECT_TRUE(connected);
	EXPECT_FALSE(error);
}

TEST_F(Client, TCPConnectFail)
{
	typedef transport_type_traits< transport_type::tcp > used_transport;
	current_transport = used_transport::value;
	add_args.push_back("--validate-message");
	StartPartner();

	ASSERT_NE(0, child_.pid);
	ASSERT_EQ(used_transport::value, endpoint_.transport());
	ASSERT_NE(0, endpoint_.get< used_transport::endpoint_data >().port);
	StopPartner();

	connection c(io_svc);

	bool connected = false;
	bool error = false;

	c.connect_async(endpoint_,
	[&](){
		connected = true;
		c.close();
	},
	[&](std::exception_ptr) {
		error = true;
	});

	io_svc->run();

	EXPECT_FALSE(connected);
	EXPECT_TRUE(error);
}

TEST_F(Client, TCPConnectInvalidValidate)
{
	typedef transport_type_traits< transport_type::tcp > used_transport;
	current_transport = used_transport::value;
	add_args.insert(add_args.end(), {"--greet-message", "hello"});
	StartPartner();

	ASSERT_NE(0, child_.pid);
	ASSERT_EQ(used_transport::value, endpoint_.transport());
	ASSERT_NE(0, endpoint_.get< used_transport::endpoint_data >().port);

	connection c(io_svc);

	bool connected = false;
	bool error = false;

	c.connect_async(endpoint_,
	[&](){
		connected = true;
		c.close();
	},
	[&](std::exception_ptr) {
		error = true;
	});

	io_svc->run();

	EXPECT_FALSE(connected);
	EXPECT_TRUE(error);
}

TEST_F(Client, DISABLED_SSL)
{
	add_args.insert(add_args.end(), {
		"--cert-file", wire::test::SERVER_CERT,
		"--key-file", wire::test::SERVER_KEY,
	});
	typedef transport_type_traits< transport_type::ssl > used_transport;
	current_transport = used_transport::value;
	StartPartner();

	ASSERT_NE(0, child_.pid);
	ASSERT_EQ(used_transport::value, endpoint_.transport());
	ASSERT_NE(0, endpoint_.get< used_transport::endpoint_data >().port);

	connection c(io_svc);

	bool connected = false;
	bool error = false;

	c.connect_async(endpoint_,
	[&](){
		connected = true;
	},
	[&](std::exception_ptr) {
		error = true;
	});

	io_svc->run();

	EXPECT_TRUE(connected);
	EXPECT_FALSE(error);
}

}  // namespace test
}  // namespace core
}  // namespace wire
