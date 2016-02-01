/*
 * connection.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_CONNECTION_HPP_
#define WIRE_CORE_CONNECTION_HPP_

#include <wire/asio_config.hpp>

#include <wire/core/endpoint.hpp>
#include <wire/core/identity.hpp>
#include <wire/core/object_fwd.hpp>
#include <wire/core/callbacks.hpp>

#include <wire/encoding/buffers.hpp>

namespace wire {
namespace core {

class connection {
public:
	/**
	 * Create connection with no endpoint.
	 */
	connection(asio_config::io_service_ptr);
	/**
	 * Create connection and start asynchronous connect.
	 * @param
	 */
	connection(asio_config::io_service_ptr, endpoint const&,
			callbacks::void_callback = nullptr,
			callbacks::exception_callback = nullptr);

	/**
	 * Start asynchronous connect
	 * @param endpoint
	 * @param cb Connected callback
	 * @param ecb Exception callback
	 */
	void
	connect_async(endpoint const&,
			callbacks::void_callback = nullptr,
			callbacks::exception_callback = nullptr);

	void
	close();

	endpoint const&
	remote_endpoint() const;

	template < typename ... T, typename ... Args >
	void
	invoke_async(identity const& id, std::string const& op,
			callbacks::callback<  T const& ... > response,
			callbacks::exception_callback exception,
			callbacks::callback< bool > sent,
			Args const& ... params)
	{
		encoding::buffer out;
		write(std::back_inserter(out), params ...);
		invoke_async(id, op, std::move(out) /** @todo invocation handlers */);
	}

	template < typename ... Args >
	void
	invoke_async(identity const& id, std::string const& op,
			callbacks::void_callback response,
			callbacks::exception_callback exception,
			callbacks::callback< bool > sent,
			Args const& ... params)
	{
		encoding::buffer out;
		write(std::back_inserter(out), params ...);
		invoke_async(id, op, std::move(out) /** @todo invocation handlers */);
	}

	void
	invoke_async(identity const&, std::string const& op,
			encoding::buffer&& /** @todo invocation handlers */);

private:
	connection(connection const&) = delete;
	connection&
	operator = (connection const&) = delete;
private:
	struct impl;
	typedef std::shared_ptr<impl> pimpl;
	pimpl pimpl_;
};

}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_CONNECTION_HPP_ */
