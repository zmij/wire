/*
 * sent_multiple_test.cpp
 *
 *  Created on: 16 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>

#include <wire/asio_config.hpp>
#include <wire/core/adapter.hpp>
#include <wire/core/connector.hpp>
#include <wire/core/connection.hpp>
#include <wire/core/object.hpp>

#include <thread>
#include <atomic>

#include "test/ping_pong.hpp"

namespace wire {
namespace test {

class notify_client : public ::test::notify {
public:
    using cb_type = ::std::function<void(::std::int32_t v)>;
public:
    notify_client(cb_type cb) : cb_{cb} {}
    void
    send_int(::std::int32_t val,
            ::wire::core::functional::void_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) override
    {
        if (cb_) {
            cb_(val);
        }
    }
private:
    cb_type cb_;
};

TEST(Connection, SendMulti)
{
    asio_config::io_service_ptr io_svc =
            ::std::make_shared< asio_config::io_service >();
    auto connector = core::connector::create_connector(io_svc);
    auto adapter = connector->create_adapter( core::identity::random(),
            { core::endpoint::tcp("127.0.0.1", 0) });
    adapter->activate();
    auto endpoints = adapter->published_endpoints();
    ASSERT_LT(0, endpoints.size());
    auto ep = endpoints.front();
    ::std::cerr << "Endpoint " << ep << "\n";

    ::std::atomic< ::std::size_t > notify_cnt{0};
    ::std::atomic< ::std::int32_t > sum{0};

    auto obj = ::std::make_shared< notify_client >(
        [io_svc, &notify_cnt, &sum](::std::int32_t v)
        {
            ::std::cerr << "Notification received " << v << "\n";
            sum += v;
            if (++notify_cnt >= 2)
                io_svc->stop();
        });
    adapter->add_object({"obj_a"}, obj);
    adapter->add_object({"obj_b"}, obj);

    ::std::thread t{[io_svc](){ io_svc->run(); }};

    auto conn = connector->get_outgoing_connection(ep);
    encoding::outgoing out{ connector };
    encoding::multiple_targets targets{ { {"obj_a"}, {} }, { {"obj_b"}, {} } };
    EXPECT_EQ(2, targets.size());

    conn->send(
        targets,
        "wire_ping", core::no_context,
        core::invocation_options{}, ::std::move(out),
        [](::std::exception_ptr ex)
        {
            ::std::cerr << "Send exception\n";
        },
        [](bool sent)
        {
            ::std::cerr << "Request sent\n";
        });

    conn->send(targets, "send_int", core::no_context, core::invocation_options{},
        [](::std::exception_ptr ex)
        {
            ::std::cerr << "Send exception\n";
        },
        [](bool sent)
        {
            ::std::cerr << "Request sent\n";
        }, 42);
    t.join();
    EXPECT_EQ(2, notify_cnt);
    EXPECT_EQ(84, sum);
}

} // namespace test
}  /* namespace wire */
