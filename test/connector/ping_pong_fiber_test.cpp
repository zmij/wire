/*
 * ping_pong_fiber_test.cpp
 *
 *  Created on: Mar 25, 2017
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <test/ping_pong.hpp>
#include <wire/core/connector.hpp>
#include <wire/core/connection.hpp>
#include "sparring/sparring_test.hpp"

#include <boost/fiber/all.hpp>
#include <pushkin/asio/fiber/shared_work.hpp>
#include <thread>
#include <vector>

namespace wire {
namespace test {

namespace this_fiber = ::boost::this_fiber;

class FiberPingPong : public wire::test::sparring::SparringTest {
protected:
    void
    SetUp() override
    {
        connector_ = core::connector::create_connector(io_svc);
        ::boost::fibers::use_scheduling_algorithm< ::psst::asio::fiber::shared_work >( io_svc );
        StartPartner();
    }

    void
    SetupArgs(args_type& args) override
    {
        //args.push_back("--log=ping-pong-test.log");
    }

    void
    ReadSparringOutput(::std::istream& is) override
    {
        ::std::string proxy_str;
        ::std::getline(is, proxy_str);
        prx_ = connector_->string_to_proxy(proxy_str);
        ::std::cerr << "Sparring proxy object is " << *prx_ << "\n";
    }


    core::connector_ptr connector_;
    core::object_prx    prx_;
};

void
ping_fiber(core::object_prx prx)
{
    ::std::cerr << "ping_fiber start.\n";
    EXPECT_NO_THROW(prx->wire_ping());
    ::std::cerr << "ping_fiber done.\n";
}

void
checked_cast_fiber(core::object_prx prx)
{
    ::test::ping_pong_prx pp_prx;
    EXPECT_NO_THROW( pp_prx = core::checked_cast< ::test::ping_pong_proxy >(prx) );
    EXPECT_TRUE(pp_prx.get());
    ::std::cerr << "checked cast fiber done.\n";
}

TEST_F(FiberPingPong, SyncPing)
{
    using boost::fibers::fiber;
    ASSERT_NE(0, child_.pid);
    ASSERT_TRUE(connector_.get());
    ASSERT_TRUE(prx_.get());

    const auto fiber_cnt    = 10;
    const auto thread_cnt   = 4;
    ::std::atomic<int>  finish_cnt{0};

    auto test_f = [&](::boost::fibers::barrier& barrier){
        try {
            ::std::cout << std::this_thread::get_id() << " " << this_fiber::get_id()
                    << " Fiber start\n";
            ::std::cout << "Start sync get connection\n";
            auto conn = prx_->wire_get_connection();
            ::std::cout << "End sync get connection\n";
            EXPECT_TRUE(conn.get());

            ::std::cout << "Start ping the proxy\n";
            EXPECT_NO_THROW(prx_->wire_ping());
            ::std::cout << "End ping the proxy\n";

            ::test::ping_pong_prx pp_prx;
            ::std::cout << "Start checked cast\n";
            EXPECT_NO_THROW( pp_prx = core::checked_cast< ::test::ping_pong_proxy >(prx_) );
            ::std::cout << "End checked cast\n";
            EXPECT_TRUE(pp_prx.get());

            EXPECT_EQ(42, pp_prx->test_int(42));
        } catch (::std::exception const& e) {
            std::cerr << "Exception while running test " << e.what() << "\n";
        }

        ::std::cout << "Wait barrier " << ++finish_cnt << "\n";
        if (barrier.wait()) {
            ::std::cout << "Stop the io service\n";
            io_svc->stop();
        }
        ::std::cout << std::this_thread::get_id() << " " << this_fiber::get_id()
                << " Fiber exit\n";
    };

    ::std::vector<::std::thread> threads;
    threads.reserve(thread_cnt);

    boost::fibers::barrier b(fiber_cnt * thread_cnt);

    for (auto i = 0; i < thread_cnt; ++i) {
        threads.emplace_back(
        [&](){
            ::boost::fibers::use_scheduling_algorithm< ::psst::asio::fiber::shared_work >( io_svc );
            for (auto i = 0; i < fiber_cnt; ++i) {
                fiber{ test_f, ::std::ref(b) }.detach();
            }
            io_svc->run();
            ::std::cout << std::this_thread::get_id()
                    << " Thread exit\n";
        });
    }


//    ::std::cerr << "Run the io svc\n";
//    io_svc->run();
    for (auto& t : threads) {
        t.join();
    }
}

//TEST_F(FiberPingPong, CheckedCast)
//{
//    using boost::fibers::fiber;
//    ASSERT_NE(0, child_.pid);
//    ASSERT_TRUE(connector_.get());
//    ASSERT_TRUE(prx_.get());
//
//    fiber f1{ checked_cast_fiber, prx_ };
//    fiber f2{ checked_cast_fiber, prx_ };
//    f1.join();
//    f2.join();
//}

} /* namespace test */
} /* namespace wire */


