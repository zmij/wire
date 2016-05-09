/*
 * ping_pong_sparring.cpp
 *
 *  Created on: May 8, 2016
 *      Author: zmij
 */

#include <iostream>
#include "ping_pong_impl.hpp"

#include <wire/core/connector.hpp>
#include <wire/core/adapter.hpp>

int
main(int argc, char* argv[])
try {
    using namespace ::wire::core;

    ::wire::asio_config::io_service_ptr io_service =
            ::std::make_shared< ::wire::asio_config::io_service >();

    connector_ptr conn = connector::create_connector(io_service, argc, argv);
    adapter_ptr adptr = conn->create_adapter("ping_pong");

    adptr->activate();
    ::std::cerr << "Adapter active endpoints " << adptr->active_endpoints() << "\n";

    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
