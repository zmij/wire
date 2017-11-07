/*
 * ssl_options.hpp
 *
 *  Created on: 16 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_CORE_DETAIL_SSL_OPTIONS_HPP_
#define WIRE_CORE_DETAIL_SSL_OPTIONS_HPP_

#include <string>
#include <wire/core/ssl_certificate_fwd.hpp>

namespace wire {
namespace core {
namespace detail {

struct ssl_options {
    ::std::string       verify_file;
    ::std::string       cert_file;
    ::std::string       key_file;
    bool                require_peer_cert;

    ssl_verify_callback verify_func;
};

}  // namespace detail
}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_DETAIL_SSL_OPTIONS_HPP_ */
