/*
 * locator_inproc_test.cpp
 *
 *  Created on: Sep 29, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>

#include <wire/core/connector.hpp>
#include <wire/core/locator.hpp>
#include <wire/locator/locator_service.hpp>

namespace wire {
namespace test {

TEST(LocatorInProc, LookupAdapter)
{
    core::connector::args_type args {
        "--wire.locator.endpoints=tcp://0.0.0.0:0"
    };

    auto io_svc = ::std::make_shared< asio_config::io_service >();
    auto cnctr = core::connector::create_connector(io_svc, args);

    svc::locator_service loc_svc{};
    loc_svc.start(cnctr);

    auto loc = cnctr->get_locator();
    EXPECT_TRUE(loc.get()) << "Locator object in connector";
    auto reg = cnctr->get_locator_registry();
    EXPECT_TRUE(reg.get()) << "Locator registry object in connector";

    auto prx = loc->find_adapter("locator"_wire_id);
    EXPECT_TRUE(prx.get()) << "Find adapter via locator";
    EXPECT_THROW(loc->find_adapter("__never_been_there__"_wire_id), core::no_adapter);
}

TEST(LocatorInProc, ReplicatedAdapter)
{
    core::connector::args_type args {
        "--wire.locator.endpoints=tcp://0.0.0.0:0"
    };

    auto io_svc = ::std::make_shared< asio_config::io_service >();
    auto cnctr = core::connector::create_connector(io_svc, args);

    svc::locator_service loc_svc{};
    loc_svc.start(cnctr);

    auto loc = cnctr->get_locator();
    ASSERT_TRUE(loc.get()) << "Locator object in connector";
    auto reg = cnctr->get_locator_registry();
    ASSERT_TRUE(reg.get()) << "Locator registry object in connector";

    core::reference_data ref1{ "test_replicated"_wire_id, {}, {}, { "tcp://10.10.10.10:8888"_wire_ep } };
    core::reference_data ref2{ "test_replicated"_wire_id, {}, {}, { "tcp://11.11.11.11:8888"_wire_ep } };
    auto prx1 = cnctr->make_proxy(ref1);
    EXPECT_TRUE(prx1.get()) << "Created proxy is not empty";
    auto prx2 = cnctr->make_proxy(ref2);
    EXPECT_TRUE(prx2.get()) << "Created proxy is not empty";

    EXPECT_NO_THROW(reg->add_replicated_adapter(prx1))
            << "Register adapter 1";
    EXPECT_NO_THROW(reg->add_replicated_adapter(prx2))
            << "Register adapter 2";

    auto prx = loc->find_adapter(ref1.object_id);
    EXPECT_TRUE(prx.get())
            << "Added adapter is found";
    EXPECT_EQ(ref1.object_id, prx->wire_identity())
            << "Returned proxy identity";
    EXPECT_EQ(2, prx->wire_get_reference()->data().endpoints.size())
            << "Returned proxy contains both endpoints";

    EXPECT_NO_THROW(reg->remove_adapter(prx2))
            << "Adapter proxy is correctly removed";
    prx = loc->find_adapter(ref1.object_id);
    EXPECT_TRUE(prx.get())
            << "Added adapter is found";
    EXPECT_EQ(ref1.object_id, prx->wire_identity())
            << "Returned proxy identity";
    EXPECT_EQ(1, prx->wire_get_reference()->data().endpoints.size())
            << "Returned proxy contains single endpoint";
    EXPECT_EQ(ref1.endpoints, prx->wire_get_reference()->data().endpoints)
            << "Returned endpoint is one of the first reference";
    EXPECT_NO_THROW(reg->remove_adapter(prx2))
            << "Adapter proxy is correctly removed";
    EXPECT_NO_THROW(reg->remove_adapter(prx1))
            << "Adapter proxy is correctly removed";
    EXPECT_THROW(reg->remove_adapter(prx1), core::no_adapter)
            << "Adapter removal throws when no adapter exists";

    EXPECT_THROW(loc->find_adapter(ref1.object_id), core::no_adapter)
            << "No adapter data is left in the locator";
}

TEST(LocatorInProc, WellKnownObject)
{
    core::connector::args_type args {
        "--wire.locator.endpoints=tcp://0.0.0.0:0"
    };

    auto io_svc = ::std::make_shared< asio_config::io_service >();
    auto cnctr = core::connector::create_connector(io_svc, args);

    svc::locator_service loc_svc{};
    loc_svc.start(cnctr);

    auto loc = cnctr->get_locator();
    ASSERT_TRUE(loc.get()) << "Locator object in connector";
    auto reg = cnctr->get_locator_registry();
    ASSERT_TRUE(reg.get()) << "Locator registry object in connector";

    core::reference_data ref{ "well_known_object"_wire_id, {}, {}, { "tcp://10.10.10.10:8888"_wire_ep } };
    EXPECT_THROW(loc->find_object(ref.object_id), core::object_not_found)
            << "Throws a no object exception when object is not found";
    auto prx = cnctr->make_proxy(ref);
    EXPECT_NO_THROW(reg->add_well_known_object(prx))
            << "Successfully add a well-know object proxy";
    EXPECT_NO_THROW(reg->add_well_known_object(prx))
            << "Successfully replace a dead well-know object proxy";

    auto res = loc->find_object(ref.object_id);
    EXPECT_TRUE(res.get())
            << "Find a well-known object by id";
    EXPECT_EQ(ref.object_id, res->wire_identity())
            << "Well-known object has correct id";
}

}  /* namespace test */
}  /* namespace wire */
