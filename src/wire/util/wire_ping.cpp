/*
 * wire_ping.cpp
 *
 *  Created on: Oct 28, 2016
 *      Author: zmij
 */

#include <iostream>

#include <boost/program_options.hpp>
#include <wire/core/connector.hpp>
#include <wire/core/reference.hpp>
#include <wire/core/proxy.hpp>
#include <chrono>
#include <thread>

void
ping(::wire::core::object_prx prx)
{
    try {
        auto start = ::std::chrono::high_resolution_clock::now();
        ::std::cout << "Pinging " << *prx << ":\t\t";
        ::std::cout.flush();
        prx->wire_ping();
        auto end = ::std::chrono::high_resolution_clock::now();
        //::std::chrono::milliseconds dur = (end - start).;
        ::std::cout << "OK (" <<
            (end - start).count() << "ns)" << ::std::endl;
    } catch (::std::exception const& e) {
        ::std::cout << e.what() << "\n";
    } catch(...) {
        ::std::cout << "unexpected exception" << "\n";
    }
}

int
main(int argc, char* argv[])
try {
    using namespace ::wire::core;
    namespace po = ::boost::program_options;
    po::options_description desc("Program options");

    reference_data ref;
    int repeats{4};
    bool indefinite{false};

    desc.add_options()
        ("help,h", "show options description")
        ("proxy,p", po::value<reference_data>(&ref), "Proxy to ping")
        ("indefinite,t", po::bool_switch(&indefinite), "Ping until interrupted")
    ;

    auto io_svc = ::std::make_shared< ::wire::asio_config::io_service >();
    auto cnctr = connector::create_connector(io_svc, argc, argv);

    po::variables_map vm;
    cnctr->configure_options(desc, vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    if (!ref.object_id.empty()) {
        auto prx = cnctr->make_proxy(ref);
        if (indefinite) {
            while(true) {
                ping(prx);
                ::std::this_thread::sleep_for(::std::chrono::seconds{1});
            }
        } else {
            for (auto i = 0; i < repeats; ++i) {
                ping(prx);
                ::std::this_thread::sleep_for(::std::chrono::seconds{1});
            }
        }
    } else {
        throw ::std::runtime_error("Proxy to ping is not specified");
    }

    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
