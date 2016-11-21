/*
 * connection_observer.hpp
 *
 *  Created on: Nov 1, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_CONNECTION_OBSERVER_HPP_
#define WIRE_CORE_CONNECTION_OBSERVER_HPP_

#include <wire/core/connection_observer_fwd.hpp>
#include <wire/core/endpoint.hpp>
#include <wire/core/identity_fwd.hpp>
#include <wire/encoding/message.hpp>

#include <exception>

namespace wire {
namespace core {

/**
 * Interface for monitoring connection events
 */
struct connection_observer {
    using invocation_target = encoding::invocation_target;
    using operation_id = encoding::operation_specs::operation_id;
    virtual ~connection_observer() {}
    /**
     * Invoked when a connection writes bytes to it's peer
     * @param bytes number of bytes
     * @param ep peer endpoint
     */
    virtual void
    send_bytes(::std::size_t bytes, endpoint const& ep) const noexcept {};
    /**
     * Invoked when a connection receives bytes from it's peer
     * @param bytes
     * @param ep
     */
    virtual void
    receive_bytes(::std::size_t bytes, endpoint const& ep) const noexcept {};

    /**
     * Connection sends an invocation to the remote servant
     * @param id
     * @param op
     * @param ep
     */
    virtual void
    invoke_remote(invocation_target const& id, operation_id const& op,
            endpoint const& ep) const noexcept {}
    /**
     * Invocation completed successfully
     * @param id
     * @param op
     * @param ep
     */
    virtual void
    invocation_ok(invocation_target const& id, operation_id const& op,
            endpoint const& ep) const noexcept {}
    /**
     * Invocation completed with an error
     * @param id
     * @param op
     * @param ep
     * @param ex
     */
    virtual void
    invocation_error(invocation_target const& id, operation_id const& op,
            endpoint const& ep, ::std::exception_ptr ex) const noexcept {}
    /**
     * Received a request from remote client
     * @param id
     * @param op
     * @param ep
     */
    virtual void
    receive_request(invocation_target const& id, operation_id const& op,
            endpoint const& ep) const noexcept {}
    /**
     * Request processed successfully
     * @param id
     * @param op
     * @param ep
     */
    virtual void
    request_ok(invocation_target const& id, operation_id const& op,
            endpoint const& ep) const noexcept {};
    /**
     * Request processing raised an exception
     * @param id
     * @param op
     * @param ep
     * @param ex
     */
    virtual void
    request_error(invocation_target const& id, operation_id const& op,
            endpoint const& ep, ::std::exception_ptr ex) const noexcept {}
    /**
     * Upcall to a servant yielded neither result nor exception.
     * This is a sign that there is a bug in servant's code.
     * @param id
     * @param op
     * @param ep
     */
    virtual void
    request_no_response(invocation_target const& id, operation_id const& op,
            endpoint const& ep) const noexcept {}
    /**
     * Upcall to a servant yielded in more than one responses (either normal
     * or exception).
     * There is a bug in servant's code.
     * @param id
     * @param op
     * @param ep
     */
    virtual void
    request_double_response(invocation_target const& id, operation_id const& op,
            endpoint const& ep) const noexcept {}
    /**
     * Connection is connected to a peer
     * @param ep
     */
    virtual void
    connect(endpoint const& ep) const noexcept {}
    /**
     * Connection failed for some reason, e.g. connection refused, timeout,
     * unmarshal error.
     * @param ep
     * @param ex
     */
    virtual void
    connection_failure(endpoint const& ep, ::std::exception_ptr ex) const noexcept{}
    /**
     * Connection is disconnected
     * @param ep
     */
    virtual void
    disconnect(endpoint const& ep) const noexcept {}
};

}  /* namespace core */
}  /* namespace wire */


#endif /* WIRE_CORE_CONNECTION_OBSERVER_HPP_ */
