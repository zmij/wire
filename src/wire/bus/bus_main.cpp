/*
 * bus_main.cpp
 *
 *  Created on: 16 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#include <iostream>
#include <fstream>
#include <thread>

#include <boost/program_options.hpp>

#include <wire/core/connector.hpp>
#include <wire/bus/bus_service.hpp>
#include <wire/util/service_runner.hpp>

int
main(int argc, char* argv[])
try {
    using namespace ::wire::core;
    namespace svc = ::wire::svc;
    namespace util = ::wire::util;
    namespace po = ::boost::program_options;

    po::options_description desc("Program options");
    ::std::string   service_name;
    ::std::size_t   thread_count{0};
    ::std::string   pid_file;

    desc.add_options()
        ("help,h", "show options description")
        ("bus-name,n",
                po::value<::std::string>(&service_name)->default_value("wire.bus"),
                "Name for wire bus configuration section")
        ("thread-count,t",
                po::value<::std::size_t>(&thread_count)->default_value(1),
                "Count of worker theads for the bus service")
        ("pid-file",
                po::value<::std::string>(&pid_file),
                "Output pid to file")
    ;

    auto io_svc = ::std::make_shared< ::wire::asio_config::io_service >();
    auto cnctr = connector::create_connector(io_svc, argc, argv);

    po::variables_map vm;
    cnctr->configure_options(desc, vm);

    if (vm.count("help")) {
        ::std::cout << desc << "\n"; // TODO connector options, locator options
        return 0;
    }

    if (!pid_file.empty()) {
        ::std::ofstream pfile(pid_file);
        pfile << getpid() << "\n";
    }

    ::std::cerr << "Bus name " << service_name << "\n";
    svc::bus_service bus_svc{service_name};
    bus_svc.start(cnctr);

    if (thread_count > ::std::thread::hardware_concurrency()) {
        thread_count = ::std::thread::hardware_concurrency();
        #if DEBUG_OUTPUT >= 1
        ::std::cerr << "Thread count corrected to " << thread_count << "\n";
        #endif
    }

    util::service_runner srv{io_svc, [&](){ bus_svc.stop(); }};

    #if DEBUG_OUTPUT >= 1
    ::std::cerr << "Running locator service with " << thread_count << " threads\n";
    #endif
    srv.run(thread_count);
    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Standard exception " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
