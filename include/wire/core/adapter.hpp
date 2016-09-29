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
#include <wire/core/object_locator_fwd.hpp>
#include <wire/core/endpoint.hpp>
#include <wire/core/detail/configuration_options_fwd.hpp>

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
    ~adapter();

    connector_ptr
    get_connector() const;

    asio_config::io_service_ptr
    io_service() const;

    detail::adapter_options const&
    options() const;

    /**
     * Start accepting connections
     */
    void
    activate(bool postpone_reg = false);
    /**
     * Register adapter in the locator
     */
    void
    register_adapter();
    /**
     * Stop accepting connections.
     * Send close to clients, close read ends.
     * Finish dispatching current requests, then close connections.
     */
    void
    deactivate();
    /**
     * @return State of adapter.
     */
    bool
    is_active() const;

    /**
     * Adapter identity
     * @return
     */
    identity const&
    id() const;
    /**
     * @return Pseudo proxy containing adapter identity and endpoints
     */
    object_prx
    adapter_proxy() const;
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
     * Get adapter published edpoints list
     * @return
     */
    endpoint_list
    published_endpoints() const;

    /**
     * Create a proxy with given identity and properties of adapter
     * @param id
     * @return
     */
    object_prx
    create_proxy(identity const& id,
            ::std::string const& facet = ::std::string{}) const;
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
     * Add a default servant for all requests (fallback or for objects with no category)
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
    add_default_servant(::std::string const& category, object_ptr);
    /**
     * Add a fallback object locator instance
     * @param
     */
    void
    add_object_locator(object_locator_ptr);
    /**
     * Add a locator instance for a given category
     * @param category
     * @param
     */
    void
    add_object_locator(::std::string const& category, object_locator_ptr);

    /**
     * Find an object by identity.
     * If there is no exact match, search default servants.
     * TODO Fallback to object_locator
     * @param
     * @return
     */
    object_ptr
    find_object(identity const&, ::std::string const& facet = ::std::string{}) const;

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
