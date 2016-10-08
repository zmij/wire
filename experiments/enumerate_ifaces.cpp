/*
 * enumerate_ifaces.cpp
 *
 *  Created on: Sep 28, 2016
 *      Author: zmij
 */

#include <iostream>
#include <memory>
#include <ifaddrs.h>
#include <boost/asio.hpp>

int
main(int argc, char* argv[])
try {
    ifaddrs* ifaces{nullptr};
    auto rc = getifaddrs(&ifaces);
    ::std::shared_ptr<ifaddrs> ifaddr_sentry{ ifaces, [](ifaddrs* ifs){ freeifaddrs(ifs); } };
    if (rc != 0)
        throw ::std::runtime_error{"Failed to obtain interfaces list"};
    char host[NI_MAXHOST];

    for (auto iface = ifaces; iface != nullptr; iface = iface->ifa_next) {
        if (!iface->ifa_addr) {
            continue;
        }

        auto family = iface->ifa_addr->sa_family;
        if (family == AF_INET || family == AF_INET6) {
            ::std::cout << (family == AF_INET ? "ipv4" : "ipv6") << " ";
            auto s = getnameinfo(iface->ifa_addr,
                (family == AF_INET) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6),
                host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
            if (s != 0) {
                throw ::std::runtime_error{"Failed to get information on address"};
            }
            bool is_loopback = iface->ifa_flags & IFF_LOOPBACK;
            ::std::cout << host << " (" << iface->ifa_name
                    << (is_loopback ? ", loopback" : "") << ")\n";
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
