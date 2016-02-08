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

#include <memory>
#include <functional>

#define ASIO_NS ::boost::asio

namespace wire {
namespace asio_config {

typedef ASIO_NS::io_service							io_service;
typedef std::shared_ptr< io_service >				io_service_ptr;

typedef ASIO_NS::ip::tcp							tcp;
typedef ASIO_NS::ip::udp							udp;
typedef ASIO_NS::local::stream_protocol				local_socket;

typedef ASIO_NS::ip::address						address;

typedef boost::system::error_code					error_code;
typedef std::function< void(error_code const&) >	asio_callback;
typedef std::function< void(error_code const&,
		std::size_t) >								asio_rw_callback;

typedef ASIO_NS::ssl::context						ssl_context;
typedef std::shared_ptr< ssl_context >				ssl_context_ptr;

constexpr ASIO_NS::use_future_t<>					use_future;

}  // namespace asio_config
}  // namespace wire


#endif /* WIRE_ASIO_CONFIG_HPP_ */
