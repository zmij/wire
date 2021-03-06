/*
 * bus_test_inproc.cpp
 *
 *  Created on: 16 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>

#include <wire/core/connector.hpp>
#include <wire/core/object.hpp>
#include <wire/core/adapter.hpp>
#include <wire/errors/not_found.hpp>
#include <wire/locator/locator_service.hpp>
#include <wire/bus/bus.hpp>
#include <wire/bus/bus_service.hpp>

#include "bus_test_intf_impl.hpp"

#include <mutex>
#include <atomic>
#include <condition_variable>
#include <thread>

namespace wire {
namespace test {

TEST(BusInProc, StartService)
{
    core::connector::args_type args{
        "--wire.locator.endpoints=tcp://127.0.0.1:0",
        "--wire.bus.endpoints=tcp://127.0.0.1:0",
        "--wire.bus.publisher.endpoints=tcp://127.0.0.1:0",
        "--wire.bus.bus=test/bus",
        "--wire.bus.print-proxy"
    };

    auto io_svc = ::std::make_shared< asio_config::io_service >();
    auto cnctr = core::connector::create_connector(io_svc, args);

    svc::locator_service loc_svc{};
    ASSERT_NO_THROW(loc_svc.start(cnctr)) << "Successfully start locator service";

    svc::bus_service bus_svc{};
    ASSERT_NO_THROW(bus_svc.start(cnctr)) << "Successfully start bus service";

    auto bus_reg = bus_svc.registry();
    ASSERT_TRUE(bus_reg.get()) << "Bus registry proxy object";
    EXPECT_NO_THROW(bus_reg->wire_ping());
    bus::bus_prx bus;
    EXPECT_NO_THROW(bus = bus_reg->get_bus("test/bus"_wire_id))
        << "Predefined bus object";
    ASSERT_TRUE(bus.get());
    EXPECT_NO_THROW(bus->wire_type());
    auto pub = bus->get_publisher();
    ASSERT_TRUE(pub.get());
    EXPECT_NO_THROW(pub->wire_ping());
    EXPECT_THROW(pub->wire_type(), errors::no_operation)
        << "Bus publisher throws on all non-void wire calls";

    EXPECT_THROW(bus_reg->create_bus("test/bus"_wire_id), bus::bus_exists);
    EXPECT_THROW(bus_reg->get_bus("__not_there"_wire_id), bus::no_bus);
    EXPECT_NO_THROW(bus = bus_reg->create_bus("test/tram"_wire_id))
        << "Create a bus";
    ASSERT_TRUE(bus.get());
    EXPECT_NO_THROW(bus->wire_type());
    EXPECT_NO_THROW(pub = bus->get_publisher());
    ASSERT_TRUE(pub.get());
    EXPECT_NO_THROW(pub->wire_ping());
    EXPECT_THROW(pub->wire_type(), errors::no_operation)
        << "Bus publisher throws on all non-void wire calls";
}

TEST(BusInProc, ForwardInvocations)
{
    using mutex_type = ::std::mutex;
    using lock_type = ::std::unique_lock<mutex_type>;

    const int num_subscribers = 20;

    core::connector::args_type args{
        "--wire.locator.endpoints=tcp://127.0.0.1:0",
        "--wire.bus.endpoints=tcp://127.0.0.1:0",
        "--wire.bus.publisher.endpoints=tcp://127.0.0.1:0",
        "--wire.bus.bus=test/bus",
        "--wire.bus.print-proxy",
        "--test.endpoints=tcp://127.0.0.1:0"
    };

    auto io_svc = ::std::make_shared< asio_config::io_service >();
    asio_config::io_service::work work(*io_svc);

    auto cnctr = core::connector::create_connector(io_svc, args);

    ::std::thread t{ [&](){ io_svc->run(); } };

    svc::locator_service loc_svc{};
    loc_svc.start(cnctr);

    svc::bus_service bus_svc{};
    bus_svc.start(cnctr);

    auto adapter = cnctr->create_adapter("test");
    adapter->activate();

    ::std::atomic<int> ping_cnt{0};
    ::std::condition_variable cond;
    mutex_type mtx;
    auto ping_cb = [&]()
    {
        if (++ping_cnt >= num_subscribers) {
            lock_type lock{mtx};
            cond.notify_all();
        }
    };

    ::std::vector<core::object_prx> subscribers;
    for (auto i = 0; i < num_subscribers; ++i) {
        auto obj = adapter->add_object( ::std::make_shared<subscriber>(ping_cb) );
        obj = adapter->create_direct_proxy(obj->wire_identity());
        subscribers.push_back(obj);
    }

    auto bus_reg = bus_svc.registry();
    ASSERT_TRUE(bus_reg.get()) << "Bus registry proxy object";
    EXPECT_NO_THROW(bus_reg->wire_ping());
    bus::bus_prx bus;
    EXPECT_NO_THROW(bus = bus_reg->get_bus("test/bus"_wire_id))
        << "Predefined bus object";
    ASSERT_TRUE(bus.get());

    for (auto const& sub : subscribers) {
        EXPECT_NO_THROW(bus->subscribe(sub))
                << "Subscribe to a bus";
    }
    auto pub = bus->get_publisher();
    ASSERT_TRUE(pub.get());
    auto evts = core::unchecked_cast<events_proxy>(pub);
    EXPECT_NO_THROW(evts->event());

    lock_type lock{mtx};
    cond.wait_for(lock, ::std::chrono::seconds{1});
    EXPECT_EQ(num_subscribers, ping_cnt);

    io_svc->stop();
    t.join();
}

} // namespace test
}  /* namespace wire */
