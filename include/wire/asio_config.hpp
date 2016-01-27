/*
 * asio_config.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#ifndef WIRE_ASIO_CONFIG_HPP_
#define WIRE_ASIO_CONFIG_HPP_

#include <boost/asio.hpp>
#include <memory>
#include <functional>

#define ASIO_NS ::boost::asio

namespace wire {
namespace asio_config {

typedef ASIO_NS::io_service							io_service;
typedef std::shared_ptr< io_service >				io_service_ptr;
typedef ASIO_NS::ip::tcp							tcp;
typedef boost::system::error_code					error_code;
typedef std::function< void(error_code const&) >	asio_callback;

}  // namespace asio_config
}  // namespace wire


#endif /* WIRE_ASIO_CONFIG_HPP_ */