/*
 * bus_extproc_test.cpp
 *
 *  Created on: Jan 16, 2017
 *      Author: zmij
 */

#include <gtest/gtest.h>

#include <wire/core/connector.hpp>
#include <wire/core/object.hpp>
#include <wire/core/adapter.hpp>
#include <wire/core/reference.hpp>
#include <wire/core/proxy.hpp>
#include <wire/core/locator.hpp>

#include <wire/errors/not_found.hpp>
#include <wire/locator/locator_service.hpp>
#include <wire/bus/bus.hpp>

#include <thread>
#include <sstream>

#include "sparring/sparring_test.hpp"
#include "bus_test_intf_impl.hpp"
#include <atomic>
#include <condition_variable>

namespace wire {
namespace test {

class RemoteBus : public wire::test::sparring::SparringTest {
protected:
    using thread_ptr    = ::std::shared_ptr<::std::thread>;

    void
    SetUp() override
    {
        ::wire::core::connector::args_type args{
            "--wire.locator.endpoints=tcp://127.0.0.1:0",
            "--test.endpoints=tcp://127.0.0.1:0",
            "--test1.endpoints=tcp://127.0.0.1:0"
        };
        connector_ = core::connector::create_connector(io_svc, args);
        locator_service_.start(connector_);

        adapter_ = connector_->create_adapter("test");
        adapter_->activate();
        adapter1_ = connector_->create_adapter("test1");
        adapter1_->activate();

        io_thread_ = ::std::make_shared< ::std::thread >( [&](){ io_svc->run(); });

        StartPartner();
    }

    void
    TearDown() override
    {
        namespace bp = ::boost::process;
        ::kill(child_.pid, SIGINT);
        int status{0};
        ::waitpid(child_.pid, &status, 0);
        locator_service_.stop();
        io_svc->stop();
        io_thread_->join();
    }

    void
    SetupArgs(args_type& args) override
    {
        ::std::ostringstream os;
        os << *connector_->get_locator();
        args.insert(args.end(), {
            "--wire.connector.locator=" + os.str(),
            "--wire.bus.endpoints=tcp://127.0.0.1:0",
            "--wire.bus.publisher.endpoints=tcp://127.0.0.1:0",
            "--wire.bus.bus=test/bus",
            "--wire.bus.print-proxy"
        });
    }

    void
    ReadSparringOutput(::std::istream& is) override
    {
        ::std::string proxy_str;
        ::std::getline(is, proxy_str);
        auto obj = connector_->string_to_proxy(proxy_str);
        ::std::cerr << "Wire bus proxy: " << *obj << "\n";
        bus_reg_ = core::unchecked_cast< bus::bus_registry_proxy >(obj);
    }

    core::connector_ptr     connector_;
    core::adapter_ptr       adapter_;
    core::adapter_ptr       adapter1_;
    svc::locator_service    locator_service_;
    thread_ptr              io_thread_;

    bus::bus_registry_prx   bus_reg_;
};

TEST_F(RemoteBus, BusSetup)
{
    ASSERT_TRUE(bus_reg_.get()) << "Nonempty bus registry proxy";
    ASSERT_NO_THROW(bus_reg_->wire_ping()) << "Bus registry exists";

    ASSERT_TRUE(bus_reg_->wire_is_a(bus_reg_->wire_static_type_id()));

    bus::bus_prx bus;
    EXPECT_NO_THROW(bus = bus_reg_->get_bus("test/bus"_wire_id))
        << "Predefined bus object";
    ASSERT_TRUE(bus.get());

    EXPECT_THROW(bus_reg_->create_bus("test/bus"_wire_id), bus::bus_exists);
    EXPECT_THROW(bus_reg_->get_bus("__not_there"_wire_id), bus::no_bus);
    EXPECT_NO_THROW(bus = bus_reg_->create_bus("test/tram"_wire_id))
        << "Create a bus";
}

TEST_F(RemoteBus, BusForward)
{
    using mutex_type = ::std::mutex;
    using lock_type = ::std::unique_lock<mutex_type>;

    const int num_subscribers = 20;
    ASSERT_TRUE(bus_reg_.get()) << "Nonempty bus registry proxy";
    ASSERT_NO_THROW(bus_reg_->wire_ping()) << "Bus registry exists";

    ASSERT_TRUE(bus_reg_->wire_is_a(bus_reg_->wire_static_type_id()));

    bus::bus_prx bus;
    EXPECT_NO_THROW(bus = bus_reg_->get_bus("test/bus"_wire_id))
        << "Predefined bus object";
    ASSERT_TRUE(bus.get());

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
    for (auto i = 0; i < num_subscribers / 2; ++i) {
        auto obj = adapter_->add_object( ::std::make_shared<subscriber>(ping_cb) );
        obj = adapter_->create_direct_proxy(obj->wire_identity());
        subscribers.push_back(obj);
    }
    for (auto i = 0; i < num_subscribers / 2; ++i) {
        auto obj = adapter1_->add_object( ::std::make_shared<subscriber>(ping_cb) );
        obj = adapter1_->create_direct_proxy(obj->wire_identity());
        subscribers.push_back(obj);
    }

    for (auto const& sub : subscribers) {
        EXPECT_NO_THROW(bus->subscribe(sub))
                << "Subscribe to a bus";
    }
    auto pub = bus->get_publisher();
    ASSERT_TRUE(pub.get());
    auto evts = core::unchecked_cast<events_proxy>(pub)->wire_one_way();
    EXPECT_NO_THROW(evts->event());

    lock_type lock{mtx};
    cond.wait_for(lock, ::std::chrono::seconds{1});
    EXPECT_EQ(num_subscribers, ping_cnt);
}

}  /* namespace test */
}  /* namespace wire */
