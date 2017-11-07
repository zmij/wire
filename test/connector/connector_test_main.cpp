/*
 * connector_test_main.cpp
 *
 *  Created on: May 10, 2016
 *      Author: zmij
 */


#include <boost/program_options.hpp>
#include <gtest/gtest.h>

#include "ping_pong_options.hpp"


// Initialize the test suite
int
main( int argc, char* argv[] )
try {
    namespace po = boost::program_options;
    using options = wire::test::ping_pong_options;

    ::testing::InitGoogleTest(&argc, argv);

    po::options_description desc("Test options");
    options& opts = options::instance();

    desc.add_options()
        ("help,h", "show options description")
        ("sparring-partner,s",
            po::value< std::string >(&opts.sparring_partner)->required(),
            "Program to run for server counterpart")
        ("tcp.req_no",
            po::value< ::std::size_t >(&opts.tcp_req_per_thread),
            "Number of requests to run in TCP MT test")
        ("tcp.timeout",
            po::value< ::std::int64_t >(&opts.tcp_test_timeout),
            "Timeout in seconds for TCP MT test")
        ("ssl.req_no",
            po::value< ::std::size_t >(&opts.ssl_req_per_thread),
            "Number of requests to run in SSL MT test")
        ("ssl.timeout",
            po::value< ::std::int64_t >(&opts.ssl_test_timeout),
            "Timeout in seconds for SSL MT test")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    po::notify(vm);

    return RUN_ALL_TESTS();
} catch (::std::exception const& e) {
    ::std::cerr << e.what() << "\n";
    return 1;
}

