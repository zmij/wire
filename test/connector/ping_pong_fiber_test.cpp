/*
 * ping_pong_fiber_test.cpp
 *
 *  Created on: Mar 25, 2017
 *      Author: zmij
 */

#ifndef WITH_BOOST_FIBERS
#define WITH_BOOST_FIBERS
#endif

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

namespace {

const char* const alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

class thread_catalogue {
private:
    std::map<std::thread::id, std::string> names_{};
    const char* next_{ alpha };
    std::mutex mtx_{};

public:
    thread_catalogue() = default;

    std::string lookup() {
        std::unique_lock<std::mutex> lk( mtx_);
        auto this_id( std::this_thread::get_id() );
        auto found = names_.find( this_id );
        if ( found != names_.end() ) {
            return found->second;
        }
        BOOST_ASSERT( *next_);
        std::string name(1, *next_++ );
        names_[ this_id ] = name;
        return name;
    }
};

thread_catalogue thread_names;

class fiber_catalogue {
private:
    std::map<boost::fibers::fiber::id, std::string> names_{};
    unsigned next_{ 0 };
    boost::fibers::mutex mtx_{};

public:
    fiber_catalogue() = default;

    std::string lookup() {
        std::unique_lock<boost::fibers::mutex> lk( mtx_);
        auto this_id( boost::this_fiber::get_id() );
        auto found = names_.find( this_id );
        if ( found != names_.end() ) {
            return found->second;
        }
        std::ostringstream out;
        // Bake into the fiber's name the thread name on which we first
        // lookup() its ID, to be able to spot when a fiber hops between
        // threads.
        out << thread_names.lookup() << next_++;
        std::string name( out.str() );
        names_[ this_id ] = name;
        return name;
    }
};

fiber_catalogue fiber_names;

::std::ostream&
tag(::std::ostream& out)
{
    using ::boost::fibers::context;
    using context_type          = ::boost::fibers::type;

    ::std::ostream::sentry s{out};
    if (s) {
        out << thread_names.lookup() << ":"
                << ::std::setw(4) << fiber_names.lookup() << " "
                << ::std::this_thread::get_id() << " ";

//        if (context::active()->is_context(context_type::dispatcher_context)) {
//            out << "disp ";
//        } else if (context::active()->is_context(context_type::main_context)) {
//            out << "main ";
//        } else {
//            out << "work ";
//        }
    }
    return out;
}

} /* namespace  */


namespace this_fiber = ::boost::this_fiber;

class FiberPingPong : public wire::test::sparring::SparringTest {
protected:
    void
    SetUp() override
    {
        connector_ = core::connector::create_connector(io_svc);
        runner_ = ::psst::asio::fiber::use_shared_work_algorithm( io_svc );
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


    core::connector_ptr             connector_;
    core::object_prx                prx_;
    ::psst::asio::fiber::runner_ptr runner_;
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
    using ::boost::fibers::fiber;

    ASSERT_NE(0, child_.pid);
    ASSERT_TRUE(connector_.get());
    ASSERT_TRUE(prx_.get());

    const auto fiber_cnt    = 1;
    const auto thread_cnt   = 2;
    ::std::atomic<int>  finish_cnt{0};

    auto test_f = [&](::boost::fibers::barrier& barrier){
        try {
            for (auto i = 0; i < 1000; ++i) {
                tag(::std::cerr) << " Fiber start " << ::boost::this_fiber::get_id() << "\n";
                tag(::std::cerr) << "Start sync get connection\n";
                auto conn = prx_->wire_get_connection();
                tag(::std::cerr) << "End sync get connection\n";
                EXPECT_TRUE(conn.get());
            }

            tag(::std::cerr) << "Start ping the proxy\n";
            EXPECT_NO_THROW(prx_->wire_ping());
            tag(::std::cerr) << "End ping the proxy\n";

            ::test::ping_pong_prx pp_prx;
            tag(::std::cerr) << "Start checked cast\n";
            EXPECT_NO_THROW( pp_prx = core::checked_cast< ::test::ping_pong_proxy >(prx_) );
            tag(::std::cerr) << "End checked cast\n";
            EXPECT_TRUE(pp_prx.get());

            tag(::std::cerr) << "Start test int\n";
            EXPECT_EQ(42, pp_prx->test_int(42));
            tag(::std::cerr) << "End test int\n";
        } catch (::std::exception const& e) {
            tag(std::cerr) << "Exception while running test " << e.what() << "\n";
        }

        tag(::std::cerr) << "Wait barrier " << ++finish_cnt << "\n";
        if (barrier.wait()) {
//            tag(::std::cerr) << "Sleep for a while\n";
//            ::boost::this_fiber::sleep_for(::std::chrono::milliseconds{5});
            tag(::std::cerr) << "Stop the io service\n";
            io_svc->stop();
        }
        tag(::std::cerr) << " Fiber exit " << ::boost::this_fiber::get_id() << "\n";
    };

    ::std::vector<::std::thread> threads;
    threads.reserve(thread_cnt);

    boost::fibers::barrier b(fiber_cnt * thread_cnt);

    for (auto i = 0; i < thread_cnt; ++i) {
        threads.emplace_back(
        [&](){
            tag(::std::cerr) << " Thread start\n";
            auto runner = ::psst::asio::fiber::use_shared_work_algorithm( io_svc );
            for (auto i = 0; i < fiber_cnt; ++i) {
                fiber{ test_f, ::std::ref(b) }.detach();
            }
            runner->run();
            tag(::std::cerr) << " Thread exit\n";
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


