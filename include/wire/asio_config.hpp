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
#include <boost/asio/streambuf.hpp>

#include <memory>
#include <functional>

namespace asio_ns = ::boost::asio;

namespace wire {
namespace asio_config {

using io_service        = asio_ns::io_service;
using io_service_ptr    = ::std::shared_ptr< io_service >;

using tcp               = asio_ns::ip::tcp;
using udp               = asio_ns::ip::udp;
using local_socket      = asio_ns::local::stream_protocol;

using address           = asio_ns::ip::address;
using address_v4        = asio_ns::ip::address_v4;
using address_v6        = asio_ns::ip::address_v6;

using error_code        = ::boost::system::error_code;
using asio_callback     = ::std::function< void(error_code const&) >;
using asio_rw_callback  = ::std::function< void(error_code const&, std::size_t) >;

using ssl_context       = asio_ns::ssl::context;
using ssl_context_ptr   = ::std::shared_ptr< ssl_context >;

using system_timer      = asio_ns::system_timer;

constexpr asio_ns::use_future_t<> use_future;

namespace error = asio_ns::error;

template <typename Code>
error_code
make_error_code(Code c)
{
    return error::make_error_code(c);
}

::std::size_t const incoming_buffer_size = 8192;

}  // namespace asio_config
}  // namespace wire


#endif /* WIRE_ASIO_CONFIG_HPP_ */
