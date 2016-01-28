/*
 * connection_impl.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_DETAIL_CONNECTION_IMPL_HPP_
#define WIRE_CORE_DETAIL_CONNECTION_IMPL_HPP_

#include <wire/core/transport.hpp>

namespace wire {
namespace core {
namespace detail {

struct connection_impl_base {
	virtual ~connection_impl_base();

	virtual void
	connect(endpoint const&, asio_config::asio_callback) = 0;
};

template < transport_type _type >
struct connection_impl {
	typedef transport_type_traits< _type >	transport_traits;
	typedef typename transport_traits::type	transport_type;

	connection_impl(asio_config::io_service_ptr io_svc)
	{
	}

	transport_type transport_;
};

}  // namespace detail
}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_DETAIL_CONNECTION_IMPL_HPP_ */
