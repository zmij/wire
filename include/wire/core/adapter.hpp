/*
 * adapter.hpp
 *
 *  Created on: 7 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_CORE_ADAPTER_HPP_
#define WIRE_CORE_ADAPTER_HPP_

#include <wire/asio_config.hpp>
#include <wire/future_config.hpp>

#include <wire/core/object_fwd.hpp>
#include <wire/core/adapter_fwd.hpp>
#include <wire/core/connector_fwd.hpp>
#include <wire/core/identity_fwd.hpp>
#include <wire/core/proxy_fwd.hpp>
#include <wire/core/object_locator_fwd.hpp>
#include <wire/core/endpoint.hpp>
#include <wire/core/connection_observer_fwd.hpp>
#include <wire/core/current_fwd.hpp>
#include <wire/core/locator_fwd.hpp>

#include <wire/core/detail/configuration_options_fwd.hpp>
#include <wire/core/detail/dispatch_request_fwd.hpp>

#include <wire/core/invocation_options.hpp>
#include <wire/core/functional.hpp>
#include <wire/core/context.hpp>

#include <wire/core/detail/future_traits.hpp>

#include <wire/util/debug_log.hpp>

#include <string>

namespace wire {
namespace core {

namespace detail {
struct adapter_options;
}  // namespace detail

/**
 * Object adapter. An entity that listens endpoints and dispatches requests
 * to objects registered with the adapter.
 *
 * Adapter identification.
 * In an application that doesn't use locator the adapter ids don't matter as
 * all communication involves direct proxies containing endpoints.
 * When a locator is used an adapter must have an unique id and register it with
 * the locator. If the adapter id contains a category, the category names the
 * adapter's replica group. To find all replica's endpoints the locator should
 * be queried with a wildcard id (replica_name / *).
 */
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
            detail::adapter_options const& options,
            connection_observer_set const&);
public:
    ~adapter();

    connector_ptr
    get_connector() const;

    asio_config::io_service_ptr
    io_service() const;

    detail::adapter_options const&
    options() const;
    detail::ssl_options const&
    ssl_options() const;

    //@{
    /** @name Activate/deactivate adapter */
    /**
     * Start accepting connections
     */
    void
    activate(bool postpone_reg = false);
    /**
     * Stop accepting connections.
     * Send close to clients, close read ends.
     * Finish dispatching current requests, then close connections.
     */
    void
    deactivate_async(functional::void_callback  result,
            functional::exception_callback      exception,
            context_type const&                             = no_context,
            invocation_options const&           opts        = invocation_options::unspecified);

    template < template <typename> class _Promise = promise >
    auto
    deactivate_async(
            context_type const&                 ctx         = no_context,
            invocation_options const&           opts        = invocation_options::unspecified)
        -> decltype(::std::declval< _Promise<void> >().get_future())
    {
        auto promise = ::std::make_shared< _Promise<void> >();

        deactivate_async(
            [promise]()
            {
                DEBUG_LOG(3, "Deactivate result");
                promise->set_value();
            },
            [promise](::std::exception_ptr ex)
            {
                DEBUG_LOG(3, "Deactivate exception");
                promise->set_exception(ex);
            }, ctx, opts
        );

        return promise->get_future();
    }
    template < template <typename> class _Promise = promise >
    void
    deactivate(
            context_type const&                 ctx         = no_context,
            invocation_options const&           opts        = invocation_options::unspecified)
    {
        auto future = deactivate_async<_Promise>(
                ctx, opts | promise_invocation_flags<_Promise<void>>::value);
        return future.get();
    }
    //@}
    //@{
    /**
     * @name Get locator configured for the adapter (or the default locator)
     */
    void
    get_locator_async (
        functional::callback<locator_prx>   result,
        functional::exception_callback      exception   = nullptr,
        context_type const&                             = no_context,
        invocation_options const&           opts        = invocation_options::unspecified) const;

    template < template <typename> class _Promise = promise >
    auto
    get_locator_async(
        context_type const&                 ctx         = no_context,
        invocation_options const&           opts        = invocation_options::unspecified) const
        -> decltype(::std::declval<_Promise<locator_prx>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<locator_prx> >();

        get_locator_async(
            [promise](locator_prx res)
            {
                promise->set_value(res);
            },
            [promise](::std::exception_ptr ex)
            {
                promise->set_exception(ex);
            }, ctx, opts
        );

        return promise->get_future();
    }

    template < template <typename> class _Promise = promise >
    locator_prx
    get_locator(
            context_type const&         ctx     = no_context,
            invocation_options const&   opts    = invocation_options::unspecified) const
    {
        auto future = get_locator_async<_Promise>(
                ctx, opts | promise_invocation_flags<_Promise<locator_prx>>::value);
        return future.get();
    }
    //@}
    //@{
    /**
     * @name Get locator registry configured for the adapter
     * (or the default locator registry)
     */
    void
    get_locator_registry_async (
        functional::callback<locator_registry_prx>  result,
        functional::exception_callback              exception   = nullptr,
        context_type const&                                     = no_context,
        invocation_options const&                   opts        = invocation_options::unspecified) const;

    template < template <typename> class _Promise = promise >
    auto
    get_locator_registry_async(
        context_type const&                 ctx     = no_context,
        invocation_options const&           opts    = invocation_options::unspecified) const
        -> decltype(::std::declval<_Promise<locator_registry_prx>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<locator_registry_prx> >();

        get_locator_registry_async(
            [promise](locator_registry_prx res)
            {
                promise->set_value(res);
            },
            [promise](::std::exception_ptr ex)
            {
                promise->set_exception(ex);
            }, ctx, opts
        );

        return promise->get_future();
    }

    template < template <typename> class _Promise = promise >
    locator_registry_prx
    get_locator_registry(
        context_type const&         ctx     = no_context,
        invocation_options const&   opts    = invocation_options::unspecified) const
    {
        auto future = get_locator_registry_async<_Promise>(
                ctx, opts | promise_invocation_flags<_Promise<locator_registry_prx>>::value);
        return future.get();
    }
    //@}
    //@{
    /**
     * @name Register adapter in the locator
     */
    void
    register_adapter_async(
        functional::void_callback       __result,
        functional::exception_callback  __exception,
        context_type const&             ctx         = no_context,
        invocation_options const&       opts        = invocation_options::unspecified
    );
    template < template <typename> class _Promise = promise >
    auto
    register_adapter_async(
        context_type const&             ctx         = no_context,
        invocation_options const&       opts        = invocation_options::unspecified)
        -> decltype( ::std::declval<_Promise<void>>().get_future() )
    {
        auto promise = ::std::make_shared<_Promise<void>>();
        register_adapter_async(
            [promise]()
            { promise->set_value(); },
            [promise](::std::exception_ptr ex)
            { promise->set_exception(::std::move(ex)); },
            ctx, opts);
        return promise->get_future();
    }
    template < template <typename> class _Promise = promise >
    void
    register_adapter(
        context_type const&             ctx         = no_context,
        invocation_options const&       opts        = invocation_options::unspecified)
    {
        auto future = register_adapter_async<_Promise>(ctx,
                opts | promise_invocation_flags<_Promise<void>>::value);
        future.get();
    }
    //@}
    //@{
    void
    unregister_adapter_async(
        functional::void_callback       __result,
        functional::exception_callback  __exception,
        context_type const&             ctx         = no_context,
        invocation_options const&       opts        = invocation_options::unspecified
    );
    template < template <typename> class _Promise = promise >
    auto
    unregister_adapter_async(
        context_type const&             ctx         = no_context,
        invocation_options const&       opts        = invocation_options::unspecified)
        -> decltype( ::std::declval<_Promise<void>>().get_future() )
    {
        auto promise = ::std::make_shared<_Promise<void>>();
        unregister_adapter_async(
            [promise]()
            { promise->set_value(); },
            [promise](::std::exception_ptr ex)
            { promise->set_exception(::std::move(ex)); },
            ctx, opts);
        return promise->get_future();
    }
    template < template <typename> class _Promise = promise >
    void
    unregister_adapter(
        context_type const&             ctx         = no_context,
        invocation_options const&       opts        = invocation_options::unspecified)
    {
        auto future = unregister_adapter_async<_Promise>(ctx,
                opts | promise_invocation_flags<_Promise<void>>::value);
        future.get();
    }
    //@}
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
     * Get adapter published edpoints list
     * @return
     */
    endpoint_list
    published_endpoints() const;

    /**
     * Create a proxy with given identity and properties of adapter
     * If the adapter is registered with locator, will create an indirect
     * proxy. Otherwise will create a direct proxy.
     * @param id
     * @param facet
     * @return
     */
    object_prx
    create_proxy(identity const& id,
            ::std::string const& facet = ::std::string{}) const;
    /**
     * Create a proxy containing endpoints of the adapter
     * @param id
     * @param facet
     * @return
     */
    object_prx
    create_direct_proxy(identity const& id,
            ::std::string const& facet = ::std::string{}) const;
    /**
     * Create a proxy containing id of the adapter and no endpoints
     * @param id
     * @param facet
     * @return
     */
    object_prx
    create_indirect_proxy(identity const& id,
            ::std::string const& facet = ::std::string{}) const;
    /**
     * Create an indirect proxy containing replica id of the adapter
     * and no endpoints
     * @param id
     * @param facet
     * @return
     */
    object_prx
    create_replicated_proxy(identity const& id,
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
     * Remove servant object with identity
     * @param
     */
    void
    remove_object(identity const&);

    /**
     * Add a default servant for all requests (fallback or for objects with no category)
     * @param
     */
    void
    add_default_servant(object_ptr);
    /**
     * Remove the default servant for all requests
     */
    void
    remove_default_servant();
    /**
     * Add a default servant for a given category
     * @param category
     * @param
     */
    void
    add_default_servant(::std::string const& category, object_ptr);
    /**
     * Remove the default servant for the given category
     * @param category
     */
    void
    remove_default_servant(::std::string const& category);
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
    dispatch(detail::dispatch_request const& disp, current const& curr);

    bool
    is_local_endpoint(endpoint const&) const;

    void
    add_observer(connection_observer_ptr);
    void
    remove_observer(connection_observer_ptr);
    /**
     * Get connection observers set.
     * Intentionally by value
     * @return
     */
    connection_observer_set
    connection_observers() const;

    void
    connection_online(endpoint const& local, endpoint const& remote);
    void
    listen_connection_online(endpoint const& local);
    void
    connection_offline(endpoint const&);
private:
    adapter(connector_ptr c, identity const& id, detail::adapter_options const&,
            connection_observer_set const&);

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
