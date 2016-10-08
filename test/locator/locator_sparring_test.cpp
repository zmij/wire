/*
 * locator_sparring_test.cpp
 *
 *  Created on: Oct 4, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/core/connector.hpp>
#include <wire/core/reference.hpp>
#include <wire/core/proxy.hpp>

#include "sparring/sparring_test.hpp"

namespace wire {
namespace test {

class RemoteLocator : public wire::test::sparring::SparringTest {
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
            "--wire.locator.endpoints=tcp://127.0.0.1:0",
            "--wire.locator.id=test",
            "--wire.locator.adapter=test_locator",
            "--wire.locator.registry.adapter=test_locator",
            "--wire.locator.print-proxy"
        });
    }

    void
    ReadSparringOutput(::std::istream& is) override
    {
        ::std::string proxy_str;
        ::std::getline(is, proxy_str);
        ::std::cerr << "Wire locator proxy string: " << proxy_str << "\n";
        core::connector::args_type args{
            "--wire.connector.locator=" + proxy_str
        };
        connector_ = core::connector::create_connector(io_svc, args);
    }

    core::connector_ptr connector_;
};

TEST_F(RemoteLocator, LocatorSetup)
{
    ASSERT_TRUE(connector_.get()) << "Connector object";
    EXPECT_TRUE(connector_->get_locator().get())
        << "Locator proxy";
    EXPECT_TRUE(connector_->get_locator_registry().get())
        << "Locator registry proxy";
}

TEST_F(RemoteLocator, LocatorAdapter)
{
    ASSERT_TRUE(connector_.get()) << "Connector object";

    auto prx = connector_->string_to_proxy( "test@test_locator" );
    EXPECT_TRUE(prx.get()) << "Create indirect proxy";
    core::connection_ptr conn;
    EXPECT_NO_THROW(conn = prx->wire_get_connection()) << "Resolve connection";
    EXPECT_TRUE(conn.get()) << "Obtain connection";
}

}  /* namespace test */
}  /* namespace wire */
