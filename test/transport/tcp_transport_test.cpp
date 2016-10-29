/*
 * tcp_transport_test.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/core/transport.hpp>

#include "sparring/sparring_test.hpp"

namespace wire {
namespace core {
namespace test {

class TCP : public wire::test::sparring::SparringTest {
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
            "--transport", "tcp",
            "--connections", "1"
        });
    }
    void
    ReadSparringOutput(std::istream& is) override
    {
        endpoint_ = endpoint{ ReadEnpointPort< detail::tcp_endpoint_data >(is) };
    }
    endpoint        endpoint_;
};

TEST_F(TCP, ConnectAsync)
{
    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(transport_type::tcp, endpoint_.transport());
    ASSERT_NE(0, boost::get<detail::tcp_endpoint_data>(endpoint_.data()).port);
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

TEST_F(TCP, Connect)
{
    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(transport_type::tcp, endpoint_.transport());
    ASSERT_NE(0, boost::get<detail::tcp_endpoint_data>(endpoint_.data()).port);

    tcp_transport tcp(io_svc);
    EXPECT_NO_THROW(tcp.connect(endpoint_));
}

TEST_F(TCP, ConnectAsyncFail)
{
    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(transport_type::tcp, endpoint_.transport());
    ASSERT_NE(0, boost::get<detail::tcp_endpoint_data>(endpoint_.data()).port);
    StopPartner();
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

TEST_F(TCP, ConnectFail)
{
    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(transport_type::tcp, endpoint_.transport());
    ASSERT_NE(0, boost::get<detail::tcp_endpoint_data>(endpoint_.data()).port);
    StopPartner();

    tcp_transport tcp(io_svc);
    EXPECT_THROW(tcp.connect(endpoint_), errors::connection_failed);
}

TEST_F(TCP, ReadWriteAsync)
{
    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(transport_type::tcp, endpoint_.transport());
    ASSERT_NE(0, boost::get<detail::tcp_endpoint_data>(endpoint_.data()).port);

    tcp_transport tcp(io_svc);

    bool connected = false;
    size_t errors = 0;
    const std::string test_str("TestString");
    asio_ns::streambuf in_buffer;
    std::string input_str;

    tcp.connect_async(endpoint_,
    [&]( asio_config::error_code const& ec ) {
        if (!ec) {
            connected = true;
            tcp.async_write( asio_ns::buffer(test_str),
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

/*
 * gcc-5.3.1 fail
TEST_F(TCP, ReadWrite)
{
    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(transport_type::tcp, endpoint_.transport());
    ASSERT_NE(0, boost::get<detail::tcp_endpoint_data>(endpoint_.data()).port);

    tcp_transport tcp(io_svc);
    ASSERT_NO_THROW(tcp.connect(endpoint_));
    const std::string test_str("TestFutureString");
    asio_ns::streambuf in_buffer;
    std::future< std::size_t > write_f = tcp.async_write(asio_ns::buffer(test_str));
    std::future< std::size_t > read_f = tcp.async_read(in_buffer);
    io_svc->run();
    EXPECT_EQ(test_str.size(), write_f.get());
    std::string input_str;
    EXPECT_EQ(test_str.size(), read_f.get());
    std::istream is(&in_buffer);
    is >> input_str;
    EXPECT_EQ(test_str, input_str);
}
*/

}  // namespace test
}  // namespace core
}  // namespace wire
