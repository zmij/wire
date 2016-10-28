/*
 * locator_probe.cpp
 *
 *  Created on: Oct 28, 2016
 *      Author: zmij
 */

#include <iostream>

#include <boost/program_options.hpp>
#include <wire/core/connector.hpp>
#include <wire/core/locator.hpp>

int
main(int argc, char* argv[])
try {
    using namespace ::wire::core;
    namespace po = ::boost::program_options;

    identity object_id, adapter_id;

    po::options_description desc("Program options");

    desc.add_options()
        ("help,h", "show options description")
        ("object,o", po::value<identity>(&object_id), "Query locator for an object")
        ("adapter,a", po::value<identity>(&adapter_id), "Query locator for an adapter")
    ;

    auto io_svc = ::std::make_shared< ::wire::asio_config::io_service >();
    auto cnctr = connector::create_connector(io_svc, argc, argv);

    po::variables_map vm;
    cnctr->configure_options(desc, vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    auto loc = cnctr->get_locator();
    if (!loc) {
        throw ::std::runtime_error{"Locator is not configured"};
    }

    if (!object_id.empty()) {
        auto prx = loc->find_object(object_id);
        ::std::cout << *prx << "\n";
    } else if (!adapter_id.empty()) {
        auto prx = loc->find_adapter(adapter_id);
        ::std::cout << *prx << "\n";
    } else {
        std::cerr << desc << "\n";
        return 1;
    }

    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
