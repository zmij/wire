/*
 * tcp_fiber_transport_test.cpp
 *
 *  Created on: Mar 27, 2017
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/core/transport.hpp>
#include <pushkin/asio/fiber/round_robin.hpp>

#include "sparring/sparring_test.hpp"

namespace wire {
namespace core {
namespace test {

class TCPFiber : public wire::test::sparring::SparringTest {
protected:
    void
    SetUp() override
    {
        ::boost::fibers::use_scheduling_algorithm< ::psst::asio::fiber::round_robin >( io_svc );
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

TEST_F(TCPFiber, ReadWriteAsync)
{
    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(transport_type::tcp, endpoint_.transport());
    ASSERT_NE(0, boost::get<detail::tcp_endpoint_data>(endpoint_.data()).port);

    const std::string test_str("TestString");
    bool connected = false;
    ::std::size_t written{0}, received{0};
    asio_ns::streambuf in_buffer;
    std::string input_str;

    ::boost::fibers::fiber{
        [&]{
            tcp_transport tcp(io_svc);
            try {
                ::std::cerr << "Start connect\n";
                tcp.connect_async(endpoint_).get();
                ::std::cerr << "Connected\n";
                connected = true;
                ::std::cerr << "Write\n";
                written = tcp.async_write(asio_ns::buffer(test_str)).get();
                ::std::cerr << "Read\n";
                received = tcp.async_read(in_buffer).get();
                ::std::cerr << "Done\n";
                std::istream is(&in_buffer);
                is >> input_str;
            } catch (...) {
                ::std::cerr << "Exception while running test fiber\n";
            }
            tcp.close();
            io_svc->stop();
        }
    }.detach();

    io_svc->run();
    EXPECT_TRUE(connected);
    EXPECT_EQ(test_str.size(), written);
    EXPECT_EQ(test_str.size(), received);
    EXPECT_EQ(test_str, input_str);
}

} /* namespace test */
} /* namespace core */
} /* namespace wire */

