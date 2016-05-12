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
#include <wire/core/proxy_fwd.hpp>
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

    /**
     * Adapter identity
     * @return
     */
    identity const&
    id() const;
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
    object_prx
    add_object(object_ptr);
    /**
     * Add servant object with identity
     * @param
     * @param
     */
    object_prx
    add_object(identity const&, object_ptr);

    /**
     * Add a default servant for all requests
     * @param
     */
    void
    add_default_servant(object_ptr);
    /**
     * Add a default servant for a given category
     * @param category
     * @param
     */
    void
    add_default_servant(std::string const& category, object_ptr);

    /**
     * Find an object by identity.
     * If there is no exact match, search default servants.
     * TODO Fallback to object_locator
     * @param
     * @return
     */
    object_ptr
    find_object(identity const&) const;

    bool
    is_local_endpoint(endpoint const&) const;

    void
    connection_online(endpoint const& local, endpoint const& remote);
    void
    listen_connection_online(endpoint const& local);
    void
    connection_offline(endpoint const&);
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
