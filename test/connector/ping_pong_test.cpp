/*
 * ping_pong_test.cpp
 *
 *  Created on: 10 мая 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>

#include <test/ping_pong.hpp>
#include <wire/core/connector.hpp>

namespace wire {
namespace test {

TEST(PingPong, DISABLED_AsyncInvokation)
{
    asio_config::io_service_ptr io_svc =
            ::std::make_shared< asio_config::io_service >();
    core::connector_ptr cnctr = core::connector::create_connector(io_svc);

    auto obj = cnctr->string_to_proxy("ping_pong tcp://127.0.0.1:60066");
    ASSERT_TRUE(obj.get());
    auto pp_prx = core::unchecked_cast< ::test::ping_pong_proxy >(obj);
    pp_prx->test_int(42);
}

} // namespace test
}  /* namespace wire */
