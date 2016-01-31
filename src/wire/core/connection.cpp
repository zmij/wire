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
connection_impl_base::connect_async(endpoint const& ep,
		callbacks::void_callback cb, callbacks::exception_callback eb)
{
	mode_ = client;
	process_event(events::connect{ ep, cb, eb });
}

void
connection_impl_base::handle_connected(asio_config::error_code const& ec)
{
	if (!ec) {
		process_event(events::connected{});
	} else {
		process_event(events::connection_failure{
			::std::make_exception_ptr(errors::connection_failed(ec.message()))
		});
	}
}

void
connection_impl_base::send_validate_message()
{
	encoding::outgoing_ptr out = ::std::make_shared<encoding::outgoing>(encoding::message::validate);
	write_async(out);
}


void
connection_impl_base::close()
{
	process_event(events::close{});
}

void
connection_impl_base::send_close_message()
{
	encoding::outgoing_ptr out = ::std::make_shared<encoding::outgoing>(encoding::message::close);
	write_async(out);
}

void
connection_impl_base::write_async(encoding::outgoing_ptr out,
		callbacks::void_callback cb)
{
	do_write_async( out,
		std::bind(&connection_impl_base::handle_write, shared_from_this(),
				std::placeholders::_1, std::placeholders::_2, cb, out));
}

void
connection_impl_base::handle_write(asio_config::error_code const& ec, std::size_t bytes,
		callbacks::void_callback cb, encoding::outgoing_ptr out)
{
	if (!ec) {
		if (cb) cb();
	} else {
		process_event(events::connection_failure{
			::std::make_exception_ptr(errors::connection_failed(ec.message()))
		});
	}
}

void
connection_impl_base::start_read()
{
	incoming_buffer_ptr buffer = ::std::make_shared< incoming_buffer >();
	auto shared_this = shared_from_this();
	read_async(buffer);
}

void
connection_impl_base::read_async(incoming_buffer_ptr buffer)
{
	do_read_async(buffer,
		std::bind(&connection_impl_base::handle_read, shared_from_this(),
				std::placeholders::_1, std::placeholders::_2, buffer));
}

void
connection_impl_base::handle_read(asio_config::error_code const& ec, std::size_t bytes,
		incoming_buffer_ptr buffer)
{
	if (!ec) {
		using encoding::message;
		message m;
		auto b = buffer->begin();
		auto e = b + bytes;
		try {
			read(b, e, m);
			switch (m.type()) {
				case message::request:
					// Read request and dispatch it
					break;
				case message::reply:
					// Read reply, find outstanding request and dispatch it
					break;
				case message::validate: {
					if (m.size > 0) {
						throw errors::connection_failed("Invalid validate message");
					}
					process_event(events::receive_validate{});
					break;
				}
				case message::close:
					process_event(events::receive_close{});
					break;
				default:
					break;
			}
			start_read();
		} catch (...) {
			/** TODO Make in a protocol error? Can we handle it */
			process_event(events::connection_failure{
				::std::current_exception()
			});
		}
	} else {
		process_event(events::connection_failure{
			::std::make_exception_ptr(errors::connection_failed(ec.message()))
		});
	}
}

void
connection_impl_base::invoke_async(identity const& id, std::string const& op,
		encoding::outgoing&& params/** @todo invocation handlers */)
{
	using encoding::request;
	encoding::outgoing_ptr out = std::make_shared<encoding::outgoing>(encoding::message::request);
	request r{
		++request_no_,
		encoding::operation_specs{ id, "", op },
		request::normal
	};
	write(std::back_inserter(*out), r);
	out->insert_encapsulation(std::move(params));
	/** @todo register invocation handlers */
	write_async(out);
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

	void
	close()
	{
		if (connection_) {
			connection_->close();
		} /** @todo Throw exception */
	}

	void
	invoke_async(identity const& id, std::string const& op,
			encoding::outgoing&& params/** @todo invocation handlers */)
	{
		if (connection_) {
			connection_->invoke_async(id, op, std::move(params));
		} /** @todo Throw exception */
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

void
connection::close()
{
	pimpl_->close();
}

void
connection::invoke_async(identity const& id, std::string const& op,
		encoding::outgoing&& params/** @todo invocation handlers */)
{
	pimpl_->invoke_async(id, op, std::move(params));
}


}  // namespace core
}  // namespace wire
