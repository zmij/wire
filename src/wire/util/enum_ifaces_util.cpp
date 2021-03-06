/*
 * enum_ifaces_util.cpp
 *
 *  Created on: Sep 28, 2016
 *      Author: zmij
 */

#include <iostream>

#include <boost/program_options.hpp>
#include <wire/util/enumerate_interfaces.hpp>

int
main(int argc, char* argv[])
try {
    using namespace ::wire::util;
    namespace po = ::boost::program_options;
    bool no_v4{false}, no_v6{false}, no_loopback{false}, loopback_only{false},
        list_ifaces{false};
    ::std::string iface_name;

    po::options_description desc("Program options");
    desc.add_options()
        ("help,h", "Show options description")
        ("no-v4,4", po::bool_switch(&no_v4), "Don't output IPv4 addresses")
        ("no-v6,6", po::bool_switch(&no_v6), "Don't output IPv6 addresses")
        ("no-loopback,n", po::bool_switch(&no_loopback),
                "Don't output loopback addresses")
        ("loopback-only,l", po::bool_switch(&loopback_only),
                "Output only loopback addresses")
        ("interface,i", po::value<::std::string>(&iface_name),
                "Filter by interface name")
        ("list", po::bool_switch(&list_ifaces),
                "Just print a list of local INET interfaces")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }
    po::notify(vm);

    get_interface_options opts = get_interface_options::all;
    if (no_v4)
        opts = opts ^ get_interface_options::ip4;
    if (no_v6)
        opts = opts ^ get_interface_options::ip6;
    if (iface_name.empty()) {
        if (no_loopback)
            opts = opts ^ get_interface_options::loopback;
        if (loopback_only)
            opts = opts ^ get_interface_options::regular;
    }

    if (!list_ifaces) {
        auto local_addrs = get_local_addresses(opts, iface_name);
        for (auto const& addr : local_addrs) {
            ::std::cout << addr << "\n";
        }
    } else {
        auto local_ifaces = get_local_interfaces(opts);
        for (auto const& iface : local_ifaces) {
            ::std::cout << iface << "\n";
        }
    }

    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
