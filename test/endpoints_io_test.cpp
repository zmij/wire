/*
 * endpoints_io.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/core/endpoint.hpp>

namespace wire {
namespace core {
namespace test {

typedef std::vector<uint8_t> buffer_type;
typedef buffer_type::const_iterator					input_iterator;
typedef std::back_insert_iterator<buffer_type>		output_iterator;

TEST(Endpoint, DataIO)
{
	{
		buffer_type buffer;
		detail::tcp_endpoint_data ep_in { "127.0.0.1", 5678, 30000 };
		EXPECT_NO_THROW(write(std::back_inserter(buffer), ep_in));
		std::cerr << "TCP endpoint data buffer size " << buffer.size() << "\n";
		detail::tcp_endpoint_data ep_out;
		EXPECT_NE(ep_in, ep_out);
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, ep_out));
		EXPECT_EQ(ep_in, ep_out);
		EXPECT_EQ(e, b);
	}

	{
		buffer_type buffer;
		detail::ssl_endpoint_data ep_in { "127.0.0.1", 5678, 30000 };
		EXPECT_NO_THROW(write(std::back_inserter(buffer), ep_in));
		std::cerr << "SSL endpoint data buffer size " << buffer.size() << "\n";
		detail::ssl_endpoint_data ep_out;
		EXPECT_NE(ep_in, ep_out);
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, ep_out));
		EXPECT_EQ(ep_in, ep_out);
		EXPECT_EQ(e, b);
	}

	{
		buffer_type buffer;
		detail::udp_endpoint_data ep_in { "127.0.0.1", 5678 };
		EXPECT_NO_THROW(write(std::back_inserter(buffer), ep_in));
		std::cerr << "UDP endpoint data buffer size " << buffer.size() << "\n";
		detail::udp_endpoint_data ep_out;
		EXPECT_NE(ep_in, ep_out);
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, ep_out));
		EXPECT_EQ(ep_in, ep_out);
		EXPECT_EQ(e, b);
	}

	{
		buffer_type buffer;
		detail::socket_endpoint_data ep_in { "/tmp/the_endpoint" };
		EXPECT_NO_THROW(write(std::back_inserter(buffer), ep_in));
		std::cerr << "Socket endpoint data buffer size " << buffer.size() << "\n";
		detail::socket_endpoint_data ep_out;
		EXPECT_NE(ep_in, ep_out);
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, ep_out));
		EXPECT_EQ(ep_in, ep_out);
		EXPECT_EQ(e, b);
	}
}

TEST(Endpoint, DISABLED_DataVariantIO)
{
	{
		buffer_type buffer;
		endpoint::endpoint_data ep{detail::tcp_endpoint_data{ "127.0.0.1", 5678 }};
		EXPECT_NO_THROW(write(std::back_inserter(buffer), ep));
	}
}

TEST(Endpoint, Construction)
{
	{
		buffer_type buffer;
		endpoint ep{ detail::tcp_endpoint_data{ "127.0.0.1", 5678 } };
		EXPECT_EQ(transport_type::tcp, ep.transport());
		//EXPECT_NO_THROW(write(std::back_inserter(buffer), ep));
	}
	{
		buffer_type buffer;
		endpoint ep{ detail::ssl_endpoint_data{ "127.0.0.1", 5678 } };
		EXPECT_EQ(transport_type::ssl, ep.transport());
		//EXPECT_NO_THROW(write(std::back_inserter(buffer), ep));
	}
	{
		buffer_type buffer;
		endpoint ep{ detail::udp_endpoint_data{ "127.0.0.1", 5678 } };
		EXPECT_EQ(transport_type::udp, ep.transport());
		//EXPECT_NO_THROW(write(std::back_inserter(buffer), ep));
	}
	{
		buffer_type buffer;
		endpoint ep{ detail::socket_endpoint_data{ "/tmp/the_sockete" } };
		EXPECT_EQ(transport_type::socket, ep.transport());
		//EXPECT_NO_THROW(write(std::back_inserter(buffer), ep));
	}
}

}  // namespace test
}  // namespace core
}  // namespace wire

