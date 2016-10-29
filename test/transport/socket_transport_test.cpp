/*
 * socket_transport_test.cpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/core/transport.hpp>

#include "sparring/sparring_test.hpp"

namespace wire {
namespace core {
namespace test {

class Socket : public wire::test::sparring::SparringTest {
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
            "--transport", "socket",
            "--connections", "1"
        });
    }
    void
    ReadSparringOutput(std::istream& is) override
    {
        detail::socket_endpoint_data endpoint_data;
        is >> endpoint_data.path;
        std::cerr << "Sparring partner is listening to local socket " << endpoint_data.path << "\n";
        endpoint_ = endpoint{ endpoint_data };
    }
    endpoint        endpoint_;
};

TEST_F(Socket, Connect)
{
    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(transport_type::socket, endpoint_.transport());
    ASSERT_FALSE(boost::get<detail::socket_endpoint_data>(endpoint_.data()).path.empty());

    socket_transport sock(io_svc);
    bool connected = false;
    sock.connect_async(endpoint_,
    [&]( asio_config::error_code const& ec ) {
        if (!ec)
            connected = true;
        sock.close();
    });

    io_svc->run();
    EXPECT_TRUE(connected);
}

TEST_F(Socket, ConnectFail)
{
    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(transport_type::socket, endpoint_.transport());
    ASSERT_FALSE(endpoint_.get<detail::socket_endpoint_data>().path.empty());
    StopPartner();

    socket_transport sock(io_svc);
    bool connected = false;
    sock.connect_async(endpoint_,
    [&]( asio_config::error_code const& ec ) {
        if (!ec)
            connected = true;
        sock.close();
    });

    io_svc->run();
    EXPECT_FALSE(connected);
}

TEST_F(Socket, ReadWrite)
{
    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(transport_type::socket, endpoint_.transport());
    ASSERT_FALSE(endpoint_.get<detail::socket_endpoint_data>().path.empty());

    socket_transport sock(io_svc);

    bool connected = false;
    size_t errors = 0;
    const std::string test_str("TestString");
    asio_ns::streambuf in_buffer;
    std::string input_str;

    sock.connect_async(endpoint_,
    [&]( asio_config::error_code const& ec ) {
        if (!ec) {
            connected = true;
            sock.async_write( asio_ns::buffer(test_str),
            [&](asio_config::error_code const& ec, std::size_t bytes_transferred){
                if (!ec) {
                    sock.async_read(in_buffer,
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

