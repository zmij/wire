/*
 * reference_resolve_test.cpp
 *
 *  Created on: Oct 4, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>

#include <wire/core/connector.hpp>
#include <wire/core/locator.hpp>
#include <wire/core/adapter.hpp>

#include <wire/locator/locator_service.hpp>

namespace wire {
namespace test {

TEST(RefResolve, NoLocator)
{
    core::connector::args_type args {};

    auto io_svc = ::std::make_shared< asio_config::io_service >();
    auto cnctr = core::connector::create_connector(io_svc, args);

    auto test_adapter = cnctr->create_adapter( "test_adapter"_wire_id,
            { "tcp://127.0.0.1:0"_wire_ep } );
    ASSERT_TRUE(test_adapter.get()) << "Adapter created";
    test_adapter->activate();
    auto adapter_eps = test_adapter->published_endpoints();

    ::std::cout << "Test adapter endpoints " << adapter_eps << "\n";

    core::reference_data ref{
        "some_object"_wire_id, {},
        "test_adapter"_wire_id };

    core::connection_ptr conn;
    EXPECT_ANY_THROW(conn = cnctr->resolve_connection(ref))
            << "Fail to resolve";
}

TEST(RefResolve, ResolveAdapter)
{
    core::connector::args_type args {
        "--wire.locator.endpoints=tcp://127.0.0.1:0"
    };

    auto io_svc = ::std::make_shared< asio_config::io_service >();
    auto cnctr = core::connector::create_connector(io_svc, args);

    svc::locator_service loc_svc{};
    loc_svc.start(cnctr);

    auto test_adapter = cnctr->create_adapter( "test_adapter"_wire_id,
            { "tcp://127.0.0.1:0"_wire_ep } );
    ASSERT_TRUE(test_adapter.get()) << "Adapter created";
    test_adapter->activate();
    auto adapter_eps = test_adapter->published_endpoints();

    ::std::cout << "Test adapter endpoints " << adapter_eps << "\n";

    core::reference_data ref{
        "some_object"_wire_id, {},
        "test_adapter"_wire_id };

    core::connection_ptr conn;
    ASSERT_NO_THROW(conn = cnctr->resolve_connection(ref))
            << "Successfully resolve connection to adapter";
    EXPECT_TRUE(conn.get())
            << "Resolved connection object";
}

TEST(RefResolve, ResolveWellKnownObjectDirect)
{
    core::connector::args_type args {
        "--wire.locator.endpoints=tcp://127.0.0.1:0"
    };

    auto io_svc = ::std::make_shared< asio_config::io_service >();
    auto cnctr = core::connector::create_connector(io_svc, args);

    svc::locator_service loc_svc{};
    loc_svc.start(cnctr);

    auto test_adapter = cnctr->create_adapter( "test_adapter"_wire_id,
            { "tcp://127.0.0.1:0"_wire_ep } );
    ASSERT_TRUE(test_adapter.get()) << "Adapter created";
    test_adapter->activate();

    auto reg = cnctr->get_locator_registry();
    ASSERT_TRUE(reg.get()) << "Locator registry object in connector";

    auto prx = test_adapter->create_direct_proxy("some_object"_wire_id);
    ::std::cout << "Direct proxy " << *prx << "\n";
    ASSERT_TRUE(prx.get()) << "Make proxy";
    EXPECT_NO_THROW(reg->add_well_known_object(prx)) << "Register a well-known object ";

    core::reference_data ref{ "some_object"_wire_id };

    core::connection_ptr conn;
    ASSERT_NO_THROW(conn = cnctr->resolve_connection(ref))
            << "Successfully resolve connection to a well-known object";
    EXPECT_TRUE(conn.get())
            << "Resolved connection object";
}

TEST(RefResolve, ResolveWellKnownObjectIndirect)
{
    core::connector::args_type args {
        "--wire.locator.endpoints=tcp://127.0.0.1:0"
    };

    auto io_svc = ::std::make_shared< asio_config::io_service >();
    auto cnctr = core::connector::create_connector(io_svc, args);

    svc::locator_service loc_svc{};
    loc_svc.start(cnctr);

    auto test_adapter = cnctr->create_adapter( "test_adapter"_wire_id,
            { "tcp://127.0.0.1:0"_wire_ep } );
    ASSERT_TRUE(test_adapter.get()) << "Adapter created";
    test_adapter->activate();

    auto reg = cnctr->get_locator_registry();
    ASSERT_TRUE(reg.get()) << "Locator registry object in connector";

    auto prx = test_adapter->create_indirect_proxy("some_object"_wire_id);
    ::std::cout << "Indirect proxy " << *prx << "\n";
    ASSERT_TRUE(prx.get()) << "Make proxy";
    EXPECT_NO_THROW(reg->add_well_known_object(prx)) << "Register a well-known object ";

    core::reference_data ref{ "some_object"_wire_id };

    core::connection_ptr conn;
    ASSERT_NO_THROW(conn = cnctr->resolve_connection(ref))
            << "Successfully resolve connection to a well-known object";
    EXPECT_TRUE(conn.get())
            << "Resolved connection object";
}

TEST(RefResolve, UseIndirectReferenceWellKnownObject)
{
    core::connector::args_type args {
        "--wire.locator.endpoints=tcp://127.0.0.1:0"
    };

    auto io_svc = ::std::make_shared< asio_config::io_service >();
    auto cnctr = core::connector::create_connector(io_svc, args);

    svc::locator_service loc_svc{};
    loc_svc.start(cnctr);

    auto test_adapter = cnctr->create_adapter( "test_adapter"_wire_id,
            { "tcp://127.0.0.1:0"_wire_ep } );
    ASSERT_TRUE(test_adapter.get()) << "Adapter created";
    test_adapter->activate();

    auto reg = cnctr->get_locator_registry();
    ASSERT_TRUE(reg.get()) << "Locator registry object in connector";

    auto prx = test_adapter->create_proxy("some_object"_wire_id);
    ::std::cout << "Auto proxy " << *prx << "\n";
    ASSERT_TRUE(prx.get()) << "Make proxy";
    EXPECT_NO_THROW(reg->add_well_known_object(prx))
            << "Register a well-known object ";

    core::connection_ptr conn;
    EXPECT_NO_THROW(conn = prx->wire_get_connection())
            << "Resolve connection via proxy";
    EXPECT_TRUE(conn.get());
}

}  /* namespace test */
}  /* namespace wire */

