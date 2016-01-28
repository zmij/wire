/*
 * connection.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_CONNECTION_HPP_
#define WIRE_CORE_CONNECTION_HPP_

#include <wire/core/endpoint.hpp>
#include <wire/core/identity.hpp>
#include <wire/core/object_fwd.hpp>

#include <wire/encoding/buffers.hpp>

namespace wire {
namespace core {

class connection {
public:
	/**
	 * Create connection with no endpoint.
	 */
	connection();
	/**
	 * Create connection and start asynchronous connect.
	 * @param
	 */
	connection(endpoint const&);

	void
	connect(endpoint const&);

	endpoint const&
	remote_endpoint() const;

	template < typename Handler, typename ... Args >
	void
	invoke(identity const&, std::string const& op, Handler, Args const ...);

	/**
	 * Add servant object with random identity
	 */
	void
	add_object(object_ptr);
	/**
	 * Add servant object with identity
	 * @param
	 * @param
	 */
	void
	add_object(object_ptr, identity const&);
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
