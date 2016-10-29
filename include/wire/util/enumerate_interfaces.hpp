/*
 * enumerate_interfaces.hpp
 *
 *  Created on: Sep 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_UTIL_ENUMERATE_INTERFACES_HPP_
#define WIRE_UTIL_ENUMERATE_INTERFACES_HPP_

#include <wire/asio_config.hpp>
#include <string>
#include <type_traits>

namespace wire {
namespace util {

enum class get_interface_options {
    none        = 0,
    ip4         = 1,
    ip6         = 2,
    loopback    = 4,
    regular     = 8,
    version     = ip4 | ip6,
    type        = loopback | regular,
    all         = ip4 | ip6 | loopback | regular
};

inline get_interface_options
operator & (get_interface_options a, get_interface_options b)
{
    using underlying_type = ::std::underlying_type<get_interface_options>::type;
    return static_cast<get_interface_options>(
        static_cast< underlying_type >(a) & static_cast< underlying_type >(b) );
}

inline get_interface_options
operator | (get_interface_options a, get_interface_options b)
{
    using underlying_type = ::std::underlying_type<get_interface_options>::type;
    return static_cast<get_interface_options>(
        static_cast< underlying_type >(a) | static_cast< underlying_type >(b) );
}

inline get_interface_options
operator ^ (get_interface_options a, get_interface_options b)
{
    using underlying_type = ::std::underlying_type<get_interface_options>::type;
    return static_cast<get_interface_options>(
        static_cast< underlying_type >(a) ^ static_cast< underlying_type >(b) );
}

/**
 * Get local IP addresses
 * @param
 * @param iface_name
 * @return
 */
::std::vector< asio_config::address >
get_local_addresses(get_interface_options = get_interface_options::all,
        ::std::string const& iface_name = ::std::string{});
/**
 * Get local interfaces names
 * @param
 * @return
 */
::std::vector< ::std::string >
get_local_interfaces(get_interface_options = get_interface_options::all);

}  /* namespace util */
}  /* namespace wire */



#endif /* WIRE_UTIL_ENUMERATE_INTERFACES_HPP_ */
