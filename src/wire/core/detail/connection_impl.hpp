/*
 * connection_impl.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_DETAIL_CONNECTION_IMPL_HPP_
#define WIRE_CORE_DETAIL_CONNECTION_IMPL_HPP_

#include <wire/core/transport.hpp>
#include <wire/encoding/buffers.hpp>

namespace wire {
namespace core {
namespace detail {

struct connection_impl_base;
typedef std::shared_ptr< connection_impl_base > connection_impl_ptr;

struct connection_impl_base : ::std::enable_shared_from_this<connection_impl_base> {
	typedef std::array< char, 1024 > incoming_buffer;
	typedef std::shared_ptr< incoming_buffer > incoming_buffer_ptr;

	static connection_impl_ptr
	create_connection( asio_config::io_service_ptr io_svc, transport_type _type );

	virtual ~connection_impl_base() {}

	virtual bool
	is_stream_oriented() const = 0;

	void
	connect_async(endpoint const&,
			callbacks::void_callback cb, callbacks::exception_callback eb);

	void
	start_read(
			callbacks::void_callback cb, callbacks::exception_callback eb);
	void
	write_async(encoding::outgoing_ptr,
			callbacks::void_callback cb, callbacks::exception_callback eb);
	void
	read_async(incoming_buffer_ptr,
			callbacks::void_callback cb, callbacks::exception_callback eb);
private:
	virtual void
	do_connect_async(endpoint const& ep, asio_config::asio_callback cb) = 0;
	virtual void
	do_write_async(encoding::outgoing_ptr, asio_config::asio_rw_callback) = 0;
	virtual void
	do_read_async(incoming_buffer_ptr, asio_config::asio_rw_callback) = 0;
private:
	void
	read_verify_message(callbacks::void_callback cb, callbacks::exception_callback eb);
	void
	send_verify_message(callbacks::void_callback cb, callbacks::exception_callback eb);
private:
	void
	handle_connected(asio_config::error_code const& ec,
			callbacks::void_callback cb, callbacks::exception_callback eb);
	void
	handle_write(asio_config::error_code const& ec, std::size_t bytes,
			callbacks::void_callback cb, callbacks::exception_callback eb,
			encoding::outgoing_ptr);
	void
	handle_read(asio_config::error_code const& ec, std::size_t bytes,
			callbacks::void_callback cb, callbacks::exception_callback eb,
			incoming_buffer_ptr);
};

template < transport_type _type >
struct connection_impl : connection_impl_base {
	typedef transport_type_traits< _type >	transport_traits;
	typedef typename transport_traits::type	transport_type;

	connection_impl(asio_config::io_service_ptr io_svc)
		: transport_(io_svc)
	{
	}
	virtual ~connection_impl() {}

	bool
	is_stream_oriented() const override
	{ return transport_traits::stream_oriented; }

private:
	void
	do_connect_async(endpoint const& ep, asio_config::asio_callback cb) override
	{
		transport_.connect_async(ep, cb);
	}
	void
	do_write_async(encoding::outgoing_ptr, asio_config::asio_rw_callback) override
	{

	}
	void
	do_read_async(incoming_buffer_ptr buffer, asio_config::asio_rw_callback cb) override
	{
		transport_.async_read( ASIO_NS::buffer(*buffer), cb );
	}

	endpoint		endpoint_;
	transport_type	transport_;
};

}  // namespace detail
}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_DETAIL_CONNECTION_IMPL_HPP_ */
