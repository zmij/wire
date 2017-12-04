/*
 * plugins_test.cpp
 *
 *  Created on: Dec 4, 2017
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/util/plugin.hpp>

#include "plugin_config.hpp"

namespace wire {
namespace util {
namespace test {

TEST(Util, DummyPlugin)
{
    auto& mgr = plugin_manager::instance();

    ASSERT_NO_THROW(mgr.get_plugin(TEST_PLUGIN_PATH));
    auto& plg = mgr.get_plugin(TEST_PLUGIN_PATH);

    EXPECT_THROW(plg.call<void>("no_entry"), ::std::runtime_error);
    EXPECT_NO_THROW(plg.call<void>("void_entry"));
    EXPECT_EQ(42, plg.call<int>("the_answer"));
    int a{2}, b{8};
    EXPECT_EQ(a + b, plg.call<int>("int_entry", &a, &b));
}

} /* namespace test */
} /* namespace util */
} /* namespace wire */

