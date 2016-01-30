/*
 * connection.cpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#include <wire/core/connection.hpp>
#include <wire/core/detail/connection_impl.hpp>
#include <wire/encoding/message.hpp>

#include <iterator>

namespace wire {
namespace core {

namespace detail {

connection_impl_ptr
connection_impl_base::create_connection(asio_config::io_service_ptr io_svc,
		transport_type _type)
{
	switch (_type) {
		case transport_type::tcp :
			return ::std::make_shared<
					connection_impl< transport_type::tcp > >( io_svc );
		default:
			break;
	}
	throw errors::logic_error(_type, " connection is not implemented yet");
}

void
connection_impl_base::send_verify_message(callbacks::void_callback cb,
		callbacks::exception_callback eb)
{
	if (is_stream_oriented()) {
		encoding::outgoing_ptr out = ::std::make_shared<encoding::outgoing>();
		encoding::write(std::back_inserter(*out),
				encoding::message{ encoding::message::validate, 0 });
		write_async(out, cb, eb);
	} else {
		if (cb) cb();
	}
}

void
connection_impl_base::read_verify_message(callbacks::void_callback cb,
		callbacks::exception_callback eb)
{
	incoming_buffer_ptr buffer = ::std::make_shared< incoming_buffer >();
	auto shared_this = shared_from_this();
	do_read_async( buffer,
	[shared_this, buffer, cb, eb](asio_config::error_code const& ec, std::size_t bytes)
	{
		if (!ec) {
			auto b = buffer->begin();
			auto e = b + bytes;

			encoding::message m;
			try {
				encoding::read(b, e, m);
				if ( m.type() != encoding::message::validate ) {
					throw errors::connection_failed( "Unexpected message type" );
				}
				if (m.size > 0) {
					throw errors::connection_failed( "Validate message size is more than 0" );
				}
				/** @todo Validate versions */
				/** @todo Send validate message only if client */
				shared_this->send_verify_message(cb, eb);
			} catch (...) {
				if (eb) {
					eb(std::current_exception());
				}
			}
		} else {
			if (eb) {
				eb(::std::make_exception_ptr(errors::connection_failed(ec.message())));
			}
		}
	});
}

void
connection_impl_base::connect_async(endpoint const& ep,
		callbacks::void_callback cb, callbacks::exception_callback eb)
{
	do_connect_async(ep,
		std::bind( &connection_impl_base::handle_connected, shared_from_this(),
				std::placeholders::_1, cb, eb));
}

void
connection_impl_base::handle_connected(asio_config::error_code const& ec,
		callbacks::void_callback cb, callbacks::exception_callback eb)
{
	if (!ec) {
		if (is_stream_oriented()) {
			// Wait for greeting message
			read_verify_message(cb, eb);
		} else if (cb) cb();
	} else {
		if (eb) {
			eb(::std::make_exception_ptr(errors::connection_failed(ec.message())));
		}
	}
}

void
connection_impl_base::write_async(encoding::outgoing_ptr out,
		callbacks::void_callback cb, callbacks::exception_callback eb)
{
	do_write_async( out,
		std::bind(&connection_impl_base::handle_write, shared_from_this(),
				std::placeholders::_1, std::placeholders::_2, cb, eb, out));
}

void
connection_impl_base::handle_write(asio_config::error_code const& ec, std::size_t bytes,
		callbacks::void_callback cb, callbacks::exception_callback eb,
		encoding::outgoing_ptr out)
{
	if (!ec) {
		if (cb) cb();
	} else {
		if (eb) {
			eb(::std::make_exception_ptr(errors::connection_failed(ec.message())));
		}
	}
}

void
connection_impl_base::read_async(incoming_buffer_ptr buffer,
		callbacks::void_callback cb, callbacks::exception_callback eb)
{
	do_read_async(buffer,
		std::bind(&connection_impl_base::handle_read, shared_from_this(),
				std::placeholders::_1, std::placeholders::_2, cb, eb, buffer));
}

void
connection_impl_base::handle_read(asio_config::error_code const& ec, std::size_t bytes,
		callbacks::void_callback cb, callbacks::exception_callback eb,
		incoming_buffer_ptr buffer)
{

}

}  // namespace detail

struct connection::impl {
	asio_config::io_service_ptr	io_service_;
	detail::connection_impl_ptr connection_;

	impl(asio_config::io_service_ptr io_service)
		: io_service_(io_service)
	{
	}
	void
	connect_async(endpoint const& ep,
			callbacks::void_callback cb, callbacks::exception_callback eb)
	{
		if (connection_) {
			// Do something with the old connection
			// Or throw exception
		}
		connection_ = detail::connection_impl_base::create_connection(io_service_, ep.transport());
		connection_->connect_async(ep, cb, eb);
	}

};

connection::connection(asio_config::io_service_ptr io_svc)
	: pimpl_(::std::make_shared<impl>(io_svc))
{
}

connection::connection(asio_config::io_service_ptr io_svc, endpoint const& ep,
		callbacks::void_callback cb, callbacks::exception_callback eb)
	: pimpl_(::std::make_shared<impl>(io_svc))
{
	pimpl_->connect_async(ep, cb, eb);
}

void
connection::connect_async(endpoint const& ep,
		callbacks::void_callback cb, callbacks::exception_callback eb)
{
	pimpl_->connect_async(ep, cb, eb);
}

}  // namespace core
}  // namespace wire
