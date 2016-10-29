/*
 * enumerate_interfaces.cpp
 *
 *  Created on: Sep 28, 2016
 *      Author: zmij
 */

#include <wire/util/enumerate_interfaces.hpp>
// TODO Wrap in ifdefs for *NIX/Windows
#include <ifaddrs.h>
#include <netdb.h>

#include <cerrno>
#include <cstring>

#include <set>

namespace wire {
namespace util {

::std::vector< asio_config::address >
get_local_addresses(get_interface_options opts, ::std::string const& iface_name)
{
    ::std::vector< asio_config::address > result;
    if (opts == get_interface_options::none)
        return result;

    ifaddrs* ifaces{nullptr};
    auto rc = getifaddrs(&ifaces);

    // RAII for ifaddrs* object
    ::std::shared_ptr<ifaddrs> ifaddr_sentry{
        ifaces, [](ifaddrs* ifs){ freeifaddrs(ifs); }};

    if (rc != 0) {
        auto err = errno;
        // TODO Think: Make a boost::system::error_code or a std::error_code
        throw ::std::runtime_error{ ::std::strerror(err) };
    }

    char host[NI_MAXHOST];

    for (auto iface = ifaces; iface != nullptr; iface = iface->ifa_next) {
        if (!iface->ifa_addr)
            continue;
        auto family = iface->ifa_addr->sa_family;
        if (family != AF_INET && family != AF_INET6)
            continue;

        if (!iface_name.empty()) {
            // compare interface name
            if (::strcasecmp(iface_name.c_str(), iface->ifa_name) != 0)
                continue;
        }
        if (opts != get_interface_options::all) {
            if (family == AF_INET &&
                    ((opts & get_interface_options::ip4) == get_interface_options::none))
                continue;
            if (family == AF_INET6 &&
                    ((opts & get_interface_options::ip6) == get_interface_options::none))
                continue;
            if (iface_name.empty()) {
                if (iface->ifa_flags & IFF_LOOPBACK) {
                    if ((opts & get_interface_options::loopback) == get_interface_options::none)
                        continue;
                } else if ((opts & get_interface_options::regular) == get_interface_options::none)
                    continue;
            }
        }

        auto rc = getnameinfo(iface->ifa_addr,
            (family == AF_INET) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6),
            host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
        if (rc != 0) {
            auto err = errno;
            // TODO Think: Make a boost::system::error_code or a std::error_code
            throw ::std::runtime_error{ ::std::strerror(err) };
        }
        if (family == AF_INET) {
            result.emplace_back( asio_config::address_v4::from_string(host) );
        } else {
            result.emplace_back( asio_config::address_v6::from_string(host) );
        }
    }

    return result;
}

::std::vector< ::std::string >
get_local_interfaces(get_interface_options opts)
{
    ::std::vector< ::std::string > result;
    if (opts == get_interface_options::none)
        return result;

    ifaddrs* ifaces{nullptr};
    auto rc = getifaddrs(&ifaces);

    // RAII for ifaddrs* object
    ::std::shared_ptr<ifaddrs> ifaddr_sentry{
        ifaces, [](ifaddrs* ifs){ freeifaddrs(ifs); }};

    if (rc != 0) {
        auto err = errno;
        // TODO Think: Make a boost::system::error_code or a std::error_code
        throw ::std::runtime_error{ ::std::strerror(err) };
    }
    ::std::set< ::std::string > seen;
    for (auto iface = ifaces; iface != nullptr; iface = iface->ifa_next) {
        if (!iface->ifa_addr)
            continue;
        auto family = iface->ifa_addr->sa_family;
        if (family != AF_INET && family != AF_INET6)
            continue;
        if (opts != get_interface_options::all) {
            if (family == AF_INET &&
                    ((opts & get_interface_options::ip4) == get_interface_options::none))
                continue;
            if (family == AF_INET6 &&
                    ((opts & get_interface_options::ip6) == get_interface_options::none))
                continue;
            if (iface->ifa_flags & IFF_LOOPBACK) {
                if ((opts & get_interface_options::loopback) == get_interface_options::none)
                    continue;
            } else if ((opts & get_interface_options::regular) == get_interface_options::none)
                continue;
        }
        ::std::string name{iface->ifa_name};
        if (!seen.count(name)) {
            result.push_back(name);
            seen.insert(name);
        }
    }

    return result;
}

}  /* namespace util */
}  /* namespace wire */
