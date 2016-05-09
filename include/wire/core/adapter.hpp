/*
 * adapter.hpp
 *
 *  Created on: 7 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_CORE_ADAPTER_HPP_
#define WIRE_CORE_ADAPTER_HPP_

#include <wire/asio_config.hpp>

#include <wire/core/object_fwd.hpp>
#include <wire/core/adapter_fwd.hpp>
#include <wire/core/connector_fwd.hpp>
#include <wire/core/identity_fwd.hpp>
#include <wire/core/endpoint.hpp>

#include <string>

namespace wire {
namespace core {

namespace detail {
struct adapter_options;
}  // namespace detail

class adapter {
public:
    /**
     * Create an adapter with specified endpoint list
     * @param svc        I/O service object
     * @param name        adapter name
     * @param options    adapter options structure
     * @return
     */
    static adapter_ptr
    create_adapter(connector_ptr c, identity const& id,
            detail::adapter_options const& options);
public:
    connector_ptr
    get_connector() const;

    asio_config::io_service_ptr
    io_service() const;
    /**
     * Start accepting connections
     */
    void
    activate();
    /**
     * Stop accepting connections. Stop dispatching requests.
     */
    void
    deactivate();

    identity const&
    name() const;
    /**
     * Get adapter configured endpoints list
     * @return
     */
    endpoint_list const&
    configured_endpoints() const;

    /**
     * Get adapter active endpoints list
     * @return
     */
    endpoint_list
    active_endpoints() const;

    /**
     * Add servant object with random UUID
     * @param
     */
    void // TODO Return a proxy
    add_object(dispatcher_ptr);
    /**
     * Add servant object with identity
     * @param
     * @param
     */
    void // TODO Return a proxy
    add_object(identity const&, dispatcher_ptr);

    /**
     * Add a default servant for all requests
     * @param
     */
    void
    add_default_servant(dispatcher_ptr);
    /**
     * Add a default servant for a given category
     * @param category
     * @param
     */
    void
    add_default_servant(std::string const& category, dispatcher_ptr);

    dispatcher_ptr
    find_object(identity const&) const;

private:
    adapter(connector_ptr c, identity const& id, detail::adapter_options const&);

    adapter(adapter const&) = delete;
    adapter&
    operator = (adapter const&) = delete;
private:
    struct impl;
    typedef std::shared_ptr<impl> pimpl;
    pimpl pimpl_;
};

}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_ADAPTER_HPP_ */
