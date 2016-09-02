/*
 * ping_pong_test.cpp
 *
 *  Created on: 10 мая 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>

#include <test/ping_pong.hpp>
#include <wire/core/connector.hpp>

#include "sparring/sparring_test.hpp"

namespace wire {
namespace test {

class PingPong : public wire::test::sparring::SparringTest {
protected:
    void
    SetUp() override
    {
        connector_ = core::connector::create_connector(io_svc);
        StartPartner();
    }

    void
    SetupArgs(args_type& args) override {}

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

namespace {

::std::string const LIPSUM_TEST_STRING = R"~(
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque malesuada
ut nulla vitae elementum. Curabitur dictum egestas mauris et accumsan. Aliquam
erat volutpat. Proin tempor enim vitae purus hendrerit, id varius tellus
malesuada. Phasellus mattis molestie est non auctor. Etiam porttitor est at
commodo mollis. Sed ut imperdiet velit. Vivamus eget sapien in lorem consequat
varius nec vitae eros. Suspendisse interdum arcu dui, eu placerat libero rutrum
quis. Nullam molestie mattis rhoncus. Duis imperdiet, massa sit amet varius
malesuada, turpis quam fringilla orci, in consequat lectus nisi non massa.
Mauris nec purus aliquam massa pellentesque finibus quis sed nisi. Nunc ac
sapien nulla. Duis volutpat dui vitae vestibulum molestie. Suspendisse sagittis
quis ex vitae pulvinar. Phasellus hendrerit in erat non convallis.

Nulla facilisi. Nunc et enim sed tortor mollis varius. Cras quis bibendum
sapien. Maecenas vitae lectus vel lorem gravida blandit. Vestibulum faucibus
sed sapien at scelerisque. Curabitur sit amet purus varius, volutpat orci id,
auctor libero. Fusce vulputate lectus sapien, eu ornare sem maximus quis. Sed
pellentesque tempus porta. Nam consequat tincidunt molestie. Quisque mollis ut
quam quis ultricies. Donec consectetur leo odio, a faucibus justo interdum et.
Vivamus fermentum justo eu sapien lobortis, non luctus nisi sodales. Donec
ullamcorper felis et justo consequat, a gravida magna interdum. Morbi at dictum
nisl. Nam condimentum bibendum turpis, eget aliquam velit vulputate vitae.
Aliquam non nibh et mi blandit varius ut nec quam.

In mattis, diam eget lobortis auctor, odio tellus tristique lectus, et sodales
odio est ut ligula. Maecenas elit odio, fermentum et leo euismod, blandit
vestibulum magna. Morbi non elementum quam. Nunc varius erat sed eros porttitor
auctor. Praesent luctus orci rutrum, fermentum nisl sit amet, varius nisi.
Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia
Curae; Mauris id turpis eget ligula consequat tincidunt. Duis sed sem ante. Nunc
fermentum risus nec orci dapibus accumsan. Maecenas vitae nunc lorem. Aenean
maximus elementum maximus. Phasellus finibus mi dolor, in molestie sem ultrices
ac.
)~";

}  /* namespace  */

TEST_F(PingPong, CheckedCast)
{
    ASSERT_NE(0, child_.pid);
    ASSERT_TRUE(connector_.get());
    ASSERT_TRUE(prx_.get());

    auto pp_prx = core::checked_cast< ::test::ping_pong_proxy >(prx_);
    EXPECT_TRUE(pp_prx.get());
    auto inv_prx = core::checked_cast< ::test::ping_pong::callback_proxy >(prx_);
    EXPECT_FALSE(inv_prx.get());
}

TEST_F(PingPong, WireFunctions)
{
    ASSERT_NE(0, child_.pid);
    ASSERT_TRUE(connector_.get());
    ASSERT_TRUE(prx_.get());

    auto pp_prx = core::checked_cast< ::test::ping_pong_proxy >(prx_);
    ASSERT_TRUE(pp_prx.get());

    EXPECT_NO_THROW(pp_prx->wire_ping());
    EXPECT_TRUE( pp_prx->wire_is_a( ::test::ping_pong_proxy::wire_static_type_id() ));
    EXPECT_FALSE( pp_prx->wire_is_a( "::foo::bar" ) );
    EXPECT_EQ( ::test::ping_pong_proxy::wire_static_type_id(), pp_prx->wire_type() );
    auto wire_types = pp_prx->wire_types();
    EXPECT_FALSE(wire_types.empty());
    EXPECT_EQ(2, wire_types.size());
}

TEST_F(PingPong, DISABLED_SyncRoundtrip)
{
    ASSERT_NE(0, child_.pid);
    ASSERT_TRUE(connector_.get());
    ASSERT_TRUE(prx_.get());

    auto pp_prx = core::checked_cast< ::test::ping_pong_proxy >(prx_);
    ASSERT_TRUE(pp_prx.get());

    auto val = pp_prx->test_int(42);
    EXPECT_EQ(42, val);
    auto str = pp_prx->test_string(LIPSUM_TEST_STRING);
    EXPECT_EQ(LIPSUM_TEST_STRING, str);

    ::test::data d{"Da Message"};
    auto data = pp_prx->test_struct(d);
    EXPECT_EQ(d, data);

    EXPECT_THROW(pp_prx->error(), ::test::oops);
    EXPECT_THROW(pp_prx->async_error(), ::test::oops);

    auto prx = connector_->string_to_proxy("events tcp://localhost:8888");
    auto cb_prx = core::unchecked_cast< ::test::ping_pong::callback_proxy >(prx);
    auto rt_prx = pp_prx->test_callback(cb_prx);
    ::std::cerr << "Proxy after roundtrip " << *rt_prx << "\n";
    EXPECT_EQ(*cb_prx, *rt_prx);


    auto ball = ::std::make_shared< ::test::named_ball >();
    ball->size = 42;
    ball->name = "Left ball";
    auto ret = pp_prx->test_ball(ball);
    ASSERT_TRUE(ret.get());
    ASSERT_EQ(ball->size, ret->size);
    auto named = ::std::dynamic_pointer_cast< ::test::named_ball >(ret);
    ASSERT_TRUE(named.get());
    ASSERT_EQ(ball->name, named->name);

    ret = pp_prx->test_ball(::test::ball_ptr{});
    ASSERT_FALSE(ret.get());
}


} // namespace test
}  /* namespace wire */
