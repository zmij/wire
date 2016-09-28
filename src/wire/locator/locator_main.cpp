/*
 * locator_main.cpp
 *
 *  Created on: Sep 27, 2016
 *      Author: zmij
 */

#include <iostream>
#include <thread>

#include <boost/program_options.hpp>

#include <wire/core/connector.hpp>
#include <wire/locator/locator_service.hpp>
#include <wire/util/service_runner.hpp>

int
main(int argc, char* argv[])
try {
    using namespace ::wire::core;
    namespace svc = ::wire::svc;
    namespace util = ::wire::util;
    namespace po = ::boost::program_options;

    po::options_description desc("Program options");
    ::std::string   locator_name;
    ::std::size_t   thread_count{0};

    desc.add_options()
        ("help,h", "show options description")
        ("locator-name,n",
                po::value<::std::string>(&locator_name)->default_value("wire.locator"),
                "Name for wire locator configuration section")
        ("thread-count,t",
                po::value<::std::size_t>(&thread_count)->default_value(1),
                "Count of worker theads for the registry")
    ;

    auto io_svc = ::std::make_shared< ::wire::asio_config::io_service >();
    auto cnctr = connector::create_connector(io_svc, argc, argv);

    po::variables_map vm;
    cnctr->configure_options(desc, vm);

    if (vm.count("help")) {
        ::std::cout << desc << "\n"; // TODO connector options, locator options
        return 0;
    }
    svc::locator_service loc_svc{locator_name};
    loc_svc.start(cnctr);

    if (thread_count > ::std::thread::hardware_concurrency()) {
        thread_count = ::std::thread::hardware_concurrency();
        #if DEBUG_OUTPUT >= 1
        ::std::cerr << "Thread count corrected to " << thread_count << "\n";
        #endif
    }

    util::service_runner srv{io_svc, [&](){ loc_svc.stop(); }};

    srv.run(thread_count);

    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
