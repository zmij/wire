/*
 * asio_ssl_config.hpp
 *
 *  Created on: Mar 26, 2017
 *      Author: zmij
 */

#ifndef WIRE_ASIO_SSL_CONFIG_HPP_
#define WIRE_ASIO_SSL_CONFIG_HPP_

#include <wire/asio_config.hpp>
#include <boost/asio/ssl.hpp>

namespace wire {
namespace asio_config {

using ssl_context       = asio_ns::ssl::context;
using ssl_context_ptr   = ::std::shared_ptr< ssl_context >;


} /* namespace asio_config */
} /* namespace wire */



#endif /* WIRE_ASIO_SSL_CONFIG_HPP_ */
