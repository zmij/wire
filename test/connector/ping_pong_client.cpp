/*
 * ping_pong_client.cpp
 *
 *  Created on: May 12, 2016
 *      Author: zmij
 */

#include <iostream>

#include <boost/program_options.hpp>
#include <wire/core/connector.hpp>
#include <wire/core/connection.hpp>
#include <test/ping_pong.hpp>

int
main(int argc, char* argv[])
try {
    using namespace ::wire::core;
    namespace po = ::boost::program_options;
    po::options_description desc("Program options");
    ::std::string proxy_str;
    desc.add_options()
        ("help,h", "show options description")
        ("server-proxy,p",
                po::value< ::std::string >(&proxy_str)->required(),
                "Ping-pong proxy string")
    ;

    po::parsed_options parsed_cmd =
            po::command_line_parser(argc, argv).options(desc).
                        allow_unregistered().run();
    po::variables_map vm;
    po::store(parsed_cmd, vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }
    auto unrecognized_cmd = po::collect_unrecognized(parsed_cmd.options, po::exclude_positional);
    po::notify(vm);

    ::wire::asio_config::io_service_ptr io_svc =
            ::std::make_shared< ::wire::asio_config::io_service >();
    ::wire::asio_config::io_service::work w{*io_svc};

    connector_ptr cr = connector::create_connector(io_svc, unrecognized_cmd);
    object_prx obj_prx = cr->string_to_proxy(proxy_str);
    ::test::ping_pong_prx pp_prx = checked_cast< ::test::ping_pong_proxy >(obj_prx);

    pp_prx->test_string("blabla");
    pp_prx->wire_get_connection()->close();

    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
