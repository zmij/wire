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

namespace {

struct {
    ::std::size_t               sent    = 0;
    ::std::size_t               ok      = 0;
    ::std::size_t               fail    = 0;
    ::std::chrono::nanoseconds  roundtrip_time{0};
} ping_stats;

}  /* namespace  */

struct time_printer {
    ::std::chrono::nanoseconds ns;
};

::std::ostream&
operator << (::std::ostream& os, time_printer const& tp)
{
    if (tp.ns.count() > 1000000) {
        ::std::chrono::duration<double, ::std::milli> ms = tp.ns;
        os << ms.count() << "ms";
    } else if (tp.ns.count() > 10000) {
        ::std::chrono::duration<double, ::std::micro> mks = tp.ns;
        os << mks.count() << "µs";
    } else {
        os << tp.ns.count() << "ns";
    }
    return os;
}

void
ping(::wire::core::object_prx prx)
{
    try {
        ::std::cout << "Pinging " << *prx << ":\t\t";
        ::std::cout.flush();
        ++ping_stats.sent;
        auto start = ::std::chrono::high_resolution_clock::now();
        prx->wire_ping();
        auto end = ::std::chrono::high_resolution_clock::now();
        ++ping_stats.ok;
        ::std::chrono::nanoseconds ns = end - start;
        ping_stats.roundtrip_time += ns;
        ::std::cout << "OK (" << time_printer{ns} << ")" << ::std::endl;
    } catch (::std::exception const& e) {
        ::std::cout << e.what() << "\n";
        ++ping_stats.fail;
    } catch(...) {
        ::std::cout << "unexpected exception" << "\n";
        ++ping_stats.fail;
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
    bool run{true};

    desc.add_options()
        ("help,h", "show options description")
        ("proxy,p", po::value<reference_data>(&ref), "Proxy to ping")
        ("count,n", po::value<int>(&repeats)->default_value(4), "Number of repeats")
        ("indefinite,t", po::bool_switch(&indefinite), "Ping until interrupted")
    ;

    auto io_svc = ::std::make_shared< ::wire::asio_config::io_service >();
    ::wire::asio_config::io_service::work work(*io_svc);
    auto cnctr = connector::create_connector(io_svc, argc, argv);

    po::variables_map vm;
    cnctr->configure_options(desc, vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    if (!ref.object_id.empty()) {
        asio_ns::signal_set signals{*io_svc};
        signals.add(SIGINT);
        signals.add(SIGTERM);
        #if defined(SIGQUIT)
        signals.add(SIGQUIT);
        #endif // defined(SIGQUIT)
        signals.async_wait(
            [&](::wire::asio_config::error_code const&, int signo)
            {
                run = false;
            });

        auto prx = cnctr->make_proxy(ref);
        if (indefinite) {
            while(run) {
                ping(prx);
                ::std::this_thread::sleep_for(::std::chrono::seconds{1});
            }
        } else {
            for (auto i = 0; i < repeats && run; ++i) {
                ping(prx);
                ::std::this_thread::sleep_for(::std::chrono::seconds{1});
            }
        }
        ::std::cout
             << "Sent:     \t"   << ping_stats.sent  << "\n"
             << "OK:       \t"     << ping_stats.ok
                 << " (" << (ping_stats.sent ?
                         (double)(ping_stats.ok) / ping_stats.sent * 100 : 0) << "%)\n"
             << "Fail:     \t"   << ping_stats.fail
                 << " (" << (ping_stats.sent ?
                         (double)(ping_stats.fail) / ping_stats.sent * 100 : 0) << "%)\n";
        if (ping_stats.ok) {
            auto avg = ping_stats.roundtrip_time / ping_stats.ok;
            ::std::cout << "Avg. time:\t" << time_printer{avg} << "\n";
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
