/*
 * asio_config.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#ifndef WIRE_ASIO_CONFIG_HPP_
#define WIRE_ASIO_CONFIG_HPP_

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/error.hpp>

#include <memory>
#include <functional>

#define ASIO_NS ::boost::asio

namespace wire {
namespace asio_config {

using io_service        = ASIO_NS::io_service;
using io_service_ptr    = ::std::shared_ptr< io_service >;

using tcp               = ASIO_NS::ip::tcp;
using udp               = ASIO_NS::ip::udp;
using local_socket      = ASIO_NS::local::stream_protocol;

using address           = ASIO_NS::ip::address;
using address_v4        = ASIO_NS::ip::address_v4;
using address_v6        = ASIO_NS::ip::address_v6;

using error_code        = ::boost::system::error_code;
using asio_callback     = ::std::function< void(error_code const&) >;
using asio_rw_callback  = ::std::function< void(error_code const&, std::size_t) >;

using ssl_context       = ASIO_NS::ssl::context;
using ssl_context_ptr   = ::std::shared_ptr< ssl_context >;

using system_timer      = ASIO_NS::system_timer;

constexpr ASIO_NS::use_future_t<> use_future;

namespace error = ASIO_NS::error;

template <typename Code>
error_code
make_error_code(Code c)
{
    return error::make_error_code(c);
}

}  // namespace asio_config
}  // namespace wire


#endif /* WIRE_ASIO_CONFIG_HPP_ */
