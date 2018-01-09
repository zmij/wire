/*
 * adapter.cpp
 *
 *  Created on: 7 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/core/adapter.hpp>
#include <wire/core/connection.hpp>
#include <wire/core/connector.hpp>
#include <wire/core/proxy.hpp>
#include <wire/core/object_locator.hpp>
#include <wire/core/locator.hpp>
#include <wire/core/connection_observer.hpp>

#include <wire/core/detail/configuration_options.hpp>
#include <wire/errors/not_found.hpp>

#include <wire/util/io_service_wait.hpp>
#include <wire/util/debug_log.hpp>

#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_unordered_set.h>

#include <boost/thread/shared_mutex.hpp>

#include <mutex>
#include <iostream>

namespace wire {
namespace core {

namespace {

::std::string const DEFAULT_CATEGORY{};

}  /* namespace  */

struct adapter::impl : ::std::enable_shared_from_this<impl> {
    struct registry_disconnect_observer : connection_observer {
        ::std::weak_ptr<impl>   adapter_;

        registry_disconnect_observer(::std::shared_ptr<impl> a)
            : adapter_{a} {}
        virtual void
        disconnect(endpoint const& ep) const noexcept override
        {
            auto adp = adapter_.lock();
            if (adp) {
                adp->registry_disconnected();
            }
        }
    };

    using connections           = ::tbb::concurrent_hash_map< endpoint, connection_ptr >;
    using active_objects        = ::tbb::concurrent_hash_map< identity, object_ptr >;
    using default_servants      = ::tbb::concurrent_hash_map< ::std::string, object_ptr >;
    using object_locators       = ::tbb::concurrent_hash_map< ::std::string, object_locator_ptr >;
    using concurrent_enpoints   = ::tbb::concurrent_unordered_set<endpoint>;

    using atomic_bool           = ::std::atomic<bool>;

    using mutex_type            = ::std::mutex;
    using lock_guard            = ::std::lock_guard<mutex_type>;
    using shared_mutex_type     = ::boost::shared_mutex;
    using shared_lock           = ::boost::shared_lock<shared_mutex_type>;
    using exclusive_lock        = ::std::lock_guard<shared_mutex_type>;

    using watchdog_ptr          = ::std::shared_ptr<registry_disconnect_observer>;

    connector_weak_ptr          connector_;
    asio_config::io_service_ptr io_service_;
    identity                    id_;

    detail::adapter_options     options_;
    invocation_options          register_options_;
    concurrent_enpoints         published_endpoints_;

    bool                        is_active_;
    atomic_bool                 registered_;

    connections                 connections_;
    active_objects              active_objects_;
    default_servants            default_servants_;
    object_locators             locators_;

    adapter_weak_ptr            owner_;

    mutex_type                  mtx_;

    shared_mutex_type           observer_mtx_;
    connection_observer_set     connection_observers_;

    watchdog_ptr                watchdog_;

    impl(connector_ptr c, identity const& id, detail::adapter_options const& options,
            connection_observer_set const& observers)
        : connector_{c}, io_service_{c->io_service()},
          id_{id}, options_{options},
          register_options_{
              invocation_options{}.with_retries(
                      options.register_retries, options.retry_timeout ) },
          is_active_{false}, registered_{false},
          connection_observers_{observers}
    {
    }

    void
    activate(bool postpone_reg)
    {
        lock_guard lock{mtx_};
        auto adp = owner_.lock();
        if (adp) {
            auto cnctr = connector_.lock();
            if (!cnctr)
                // FIXME Correct exception
                throw errors::connector_destroyed{"Connector was already destroyed"};
            if (options_.endpoints.empty()) {
                options_.endpoints.push_back(endpoint::tcp("0.0.0.0", 0));
            }
            for (auto const& ep : options_.endpoints) {
                connections_.emplace(ep,
                    std::make_shared<connection>(server_side{}, adp, ep ));
            }
            is_active_ = true;
            if ((options_.registered || is_replicated()) && !postpone_reg) {
                try {
                    register_adapter();
                } catch (...) {
                    if (is_replicated())
                        throw;
                }
            }
        } else {
            // FIXME Correct exception
            throw errors::adapter_destroyed{"Adapter owning implementation was destroyed"};
        }
    }

    bool
    is_replicated() const
    {
        return !id_.category.empty();
    }

    void
    get_locator_async(
            functional::callback< locator_prx > __result,
            functional::exception_callback      __exception,
            context_type const&                 ctx,
            invocation_options                  opts)
    {
        auto cnctr = connector_.lock();
        if (!cnctr)
            return functional::report_exception(__exception,
                    errors::connector_destroyed{"Connector was already destroyed"});

        if (opts == invocation_options::unspecified) {
            opts = register_options_;
        }

        if (options_.locator_ref.valid()) {
            // Use locator configured for the adapter
            cnctr->get_locator_async(options_.locator_ref,
                    __result, __exception, ctx, opts);
        } else {
            // Use connector's default adapter
            cnctr->get_locator_async(
                    __result, __exception, ctx, opts);
        }
    }

    void
    get_locator_registry_async(
            functional::callback< locator_registry_prx >    __result,
            functional::exception_callback                  __exception,
            context_type const&                             ctx,
            invocation_options                              opts)
    {
        auto cnctr = connector_.lock();
        if (!cnctr)
            return functional::report_exception(__exception,
                    errors::connector_destroyed{"Connector was already destroyed"});

        if (opts == invocation_options::unspecified) {
            opts = register_options_;
        }


        if (options_.locator_ref.valid()) {
            // Use locator configured for the adapter
            cnctr->get_locator_registry_async(options_.locator_ref,
                    __result, __exception, ctx, opts);
        } else {
            // Use connector's default adapter
            cnctr->get_locator_registry_async(
                    __result, __exception, ctx, opts);
        }
    }

    void
    registry_disconnected()
    {
        if (registered_) {
            registered_ = false;
            register_adapter_async([](){}, [](::std::exception_ptr){},
                no_context, register_options_.with_retries(
                        invocation_options::infinite_retries,
                        register_options_.retry_timeout));
        }
    }

    void
    set_registered(locator_registry_prx reg)
    {
        registered_ = true;
        // Subscribe reconnect
        if (reg) {
            if (!watchdog_)
                watchdog_ =
                    ::std::make_shared<registry_disconnect_observer>(shared_from_this());
            reg->wire_get_connection()->add_observer(watchdog_);
        }
    }

    void
    set_unregistered(locator_registry_prx reg)
    {
        registered_ = false;
        if (reg && watchdog_) {
            reg->wire_get_connection()->remove_observer(watchdog_);
        }
    }

    void
    register_adapter()
    {
        auto future = register_adapter_async(no_context, register_options_ | invocation_flags::sync);
        future.get();
    }

    void
    register_adapter_async(
            functional::void_callback       __result,
            functional::exception_callback  __exception,
            context_type const&             ctx         = no_context,
            invocation_options const&       opts        = invocation_options::unspecified)
    {
        if (registered_)
            return __result();
        DEBUG_LOG_TAG(2, tag, "Register adapter " << id_)
        auto _this = shared_from_this();
        auto on_get_reg =
            [_this, __result, __exception, ctx, opts](locator_registry_prx reg)
            {
                if (reg) {
                    DEBUG_LOG_TAG(2, _this->tag, "Add adapter " << _this->id_ <<
                            " to registry " << *reg);
                    auto prx = _this->adapter_proxy();
                    if (_this->is_replicated()) {
                        reg->add_replicated_adapter_async(prx,
                            [_this, __result, reg]()
                            {
                                _this->set_registered(reg);
                                __result();
                            }, __exception, nullptr, ctx, opts);
                    } else {
                        reg->add_adapter_async(prx,
                            [_this, __result, reg]()
                            {
                                _this->set_registered(reg);
                                __result();
                            }, __exception, nullptr, ctx, opts);
                    }
                } else {
                    DEBUG_LOG_TAG(2, _this->tag, "No locator registry when registering adapter " << _this->id_);
                    functional::report_exception(__exception,
                        errors::runtime_error{
                            "wire.locator is not configured for registering adapter"});
                }
            };
        get_locator_registry_async(on_get_reg, __exception, ctx, opts);
    }
    template < template <typename> class _Promise = promise >
    auto
    register_adapter_async(
            context_type const&             ctx         = no_context,
            invocation_options const&       opts        = invocation_options::unspecified)
        -> decltype(::std::declval< _Promise<void> >().get_future())
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

    void
    unregister_adapter_async(
            functional::void_callback       __result,
            functional::exception_callback  __exception,
            context_type const&             ctx         = no_context,
            invocation_options const&       opts        = invocation_options::unspecified)
    {
        if (!registered_)
            return __result();
        DEBUG_LOG_TAG(2, tag, "Unregister adapter");
        auto done = ::std::make_shared< ::std::atomic<bool> >(false);
        auto _this = shared_from_this();
        auto exception = [_this, done, __exception](::std::exception_ptr ex)
                {
                    *done = true;
                    DEBUG_LOG_TAG(2, _this->tag, "Exception when unregistering adapter");
                    functional::report_exception(__exception, ex);
                };
        auto on_get_reg =
            [_this, __result, exception, ctx, done](locator_registry_prx reg)
            {
                if (reg) {
                    DEBUG_LOG_TAG(2, _this->tag, "Remove adapter " << _this->id_
                            << " from registry " << *reg);
                    _this->set_unregistered(reg);
                    auto prx = _this->adapter_proxy();
                    reg->remove_adapter_async(
                        prx, [_this, __result, done](){
                            *done = true;
                            DEBUG_LOG_TAG(2, _this->tag, "Done removing adapter from locator registry");
                            __result();
                        }, exception, nullptr, ctx, invocation_options{});
                } else {
                    *done = true;
                    DEBUG_LOG_TAG(2, _this->tag, "No locator registry to remove");
                    __result();
                }
            };
        get_locator_registry_async(on_get_reg, exception, ctx, invocation_options{});
        // Throttle sync call here
        if (opts.is_sync()) {
            DEBUG_LOG_TAG(3, tag, "Wait for unregistering");
            util::run_until(io_service_, [done](){ return (bool)*done; });
            DEBUG_LOG_TAG(3, tag, "Wait for unregistering done");
        }
    }
    template < template <typename> class _Promise = promise >
    auto
    unregister_adapter_async(
        context_type const&         ctx         = no_context,
        invocation_options const&   opts        = invocation_options::unspecified)
        -> decltype(::std::declval< _Promise<void> >().get_future())
    {
        auto promise = ::std::make_shared<_Promise<void>>();
        unregister_adapter_async(
            [promise]()
            { promise->set_value(); },
            [promise](::std::exception_ptr ex)
            { promise->set_exception(::std::move(ex)); },
            ctx, opts
        );
        return promise->get_future();
    }

    void
    unregister_adapter()
    {
        auto future = unregister_adapter_async(no_context, invocation_flags::sync);
        future.get();
    }

    bool
    is_local_endpoint(endpoint const& ep)
    {
        connections::const_accessor acc;
        return connections_.find(acc, ep);
    }

    void
    clear_connections()
    {
        lock_guard lock{mtx_};
        for (auto& c : connections_) {
            c.second->close();
        }
        connections_.clear();
        published_endpoints_.clear();
        is_active_ = false;
    }

    void
    deactivate()
    {
        try {
            unregister_adapter();
        } catch (...) { /* ignore it */}
        clear_connections();
    }


    void
    deactivate_async(functional::void_callback  __result,
            functional::exception_callback      __exception,
            context_type const&                 ctx         = no_context,
            invocation_options const&           opts        = invocation_options::unspecified)
    {
        auto _this = shared_from_this();
        unregister_adapter_async(
            [_this, __result, __exception]()
            {
                try {
                    _this->clear_connections();
                    __result();
                } catch (...) {
                    functional::report_exception(__exception, ::std::current_exception());
                }
            }, __exception, ctx, opts);
    }

    void
    add_observer(connection_observer_ptr observer)
    {
        {
            exclusive_lock lock{observer_mtx_};
            connection_observers_.insert(observer);
        }
        for (auto& c : connections_) {
            c.second->add_observer(observer);
        }
    }
    void
    remove_observer(connection_observer_ptr observer)
    {
        {
            exclusive_lock lock{observer_mtx_};
            connection_observers_.erase(observer);
        }
        for (auto& c : connections_) {
            c.second->add_observer(observer);
        }
    }
    connection_observer_set
    connection_observers()
    {
        shared_lock lock{observer_mtx_};
        return connection_observers_;
    }

    void
    connection_online(endpoint const& local, endpoint const& remote)
    {
        DEBUG_LOG_TAG(1, tag, "Connection " << local << " -> " << remote << " is online")
    }
    void
    listen_connection_online(endpoint const& ep)
    {
        auto cnctr = connector_.lock();
        if (!cnctr)
            throw errors::connector_destroyed{ "Connector was already destroyed" };
        endpoint_list eps;
        ep.published_endpoints(eps);
        published_endpoints_.insert(eps.begin(), eps.end());
        cnctr->adapter_online(owner_.lock(), ep);
    }
    void
    connection_offline(endpoint const& ep)
    {
        DEBUG_LOG_TAG(1, tag, "Connection " << ep << " is offline")
    }

    endpoint_list
    published_endpoints()
    {
        endpoint_list endpoints;
        if (!published_endpoints_.empty()) {
            endpoints = endpoint_list{ published_endpoints_.cbegin(),
                published_endpoints_.cend() };
        } else {
            for (auto const& ep : options_.endpoints) {
                ep.published_endpoints(endpoints);
            }
        }
        return endpoints;
    }

    object_prx
    adapter_proxy()
    {
        return ::std::make_shared< object_proxy >(
            reference::create_reference(
                connector_.lock(), { id_, {}, {}, published_endpoints() }));
    }
    object_prx
    create_proxy(identity const& id, ::std::string const& facet)
    {
        // TODO Use configuration of adapter
        if (registered_) {
            return create_indirect_proxy(id, facet);
        }
        return create_direct_proxy(id, facet);
    }
    object_prx
    create_direct_proxy(identity const& id, ::std::string const& facet)
    {
        return ::std::make_shared< object_proxy >(
            reference::create_reference(
                connector_.lock(), { id, facet, {}, published_endpoints() }));
    }
    object_prx
    create_indirect_proxy(identity const& id, ::std::string const& facet)
    {
        return ::std::make_shared< object_proxy >(
            reference::create_reference(
                connector_.lock(), { id, facet, id_, }));
    }
    object_prx
    create_replicated_proxy(identity const& id, ::std::string const& facet)
    {
        auto replica_id = identity::wildcard(id_.category);
        return ::std::make_shared< object_proxy >(
            reference::create_reference(
                connector_.lock(), { id, facet, replica_id, }));
    }
    object_prx
    add_object(identity const& id, object_ptr disp)
    {
        active_objects_.insert(::std::make_pair(id, disp));
        return create_proxy(id, {});
    }
    void
    remove_object(identity const& id)
    {
        active_objects::const_accessor o;
        if (active_objects_.find(o, id))
            active_objects_.erase(o);
    }

    void
    add_default_servant(::std::string const& category, object_ptr disp)
    {
        default_servants_.emplace(category, disp);
    }
    void
    remove_default_servant(::std::string const& category)
    {
        default_servants::const_accessor o;
        if (default_servants_.find(o, category))
            default_servants_.erase(o);
    }

    void
    add_locator(::std::string const& category, object_locator_ptr loc)
    {
        locators_.emplace(category, loc);
    }

    void
    remove_locator(::std::string const& category)
    {
        object_locators::const_accessor loc;
        if (locators_.find(loc, category))
            locators_.erase(loc);
    }

    object_ptr
    find_object(identity const& id, ::std::string const& facet)
    {
        active_objects::const_accessor o;
        if (active_objects_.find(o, id)) {
            return o->second;
        }

        default_servants::const_accessor d;
        if (default_servants_.find(d, id.category)) {
            return d->second;
        }
        if (!id.category.empty()) {
            if (default_servants_.find(d, DEFAULT_CATEGORY)) {
                return d->second;
            }
        }

        object_locators::const_accessor loc;
        if (locators_.find(loc, id.category)) {
            // FIXME Do it async
            auto obj = loc->second->find_object(owner_.lock(), id, facet);
            if (obj)
                return obj;
        }
        if (!id.category.empty()) {
            if (locators_.find(loc, DEFAULT_CATEGORY)) {
                // FIXME Do it async
                return loc->second->find_object(owner_.lock(), id, facet);
            }
        }

        return object_ptr{};
    }

    bool
    dispatch(detail::dispatch_request const& req, current const& curr)
    {
        auto obj = find_object(curr.operation.target.identity,
                curr.operation.target.facet);
        if (!obj)
            return false;
        obj->__dispatch(req, curr);
        return true;
    }

    ::std::ostream&
    tag(::std::ostream& os) const
    {
        auto t = ::std::time(nullptr);
        os << t << " " << ::getpid() << " "<< id_;
        return os;
    }
};

adapter_ptr
adapter::create_adapter(connector_ptr c, identity const& id,
        detail::adapter_options const& options,
        connection_observer_set const& observers)
{
    adapter_ptr a(new adapter{c, id, options, observers});
    a->pimpl_->owner_ = a;
    return a;
}

adapter::adapter(connector_ptr c, identity const& id,
        detail::adapter_options const& options,
        connection_observer_set const& observers)
    : pimpl_( ::std::make_shared<impl>(c, id, options, observers) )
{
}

adapter::~adapter() = default;

connector_ptr
adapter::get_connector() const
{
    return pimpl_->connector_.lock();
}

asio_config::io_service_ptr
adapter::io_service() const
{
    return pimpl_->io_service_;
}

detail::adapter_options const&
adapter::options() const
{
    return pimpl_->options_;
}

detail::ssl_options const&
adapter::ssl_options() const
{
    return pimpl_->options_.adapter_ssl;
}

void
adapter::activate(bool postpone_reg)
{
    pimpl_->activate(postpone_reg);
}

void
adapter::deactivate_async(functional::void_callback __result,
            functional::exception_callback          __exception,
            context_type const&                     ctx,
            invocation_options const&               opts)
{
    pimpl_->deactivate_async(__result, __exception, ctx, opts);
}

void
adapter::get_locator_async(
        functional::callback<locator_prx>   result,
        functional::exception_callback      exception,
        context_type const&                 ctx,
        invocation_options const&           opts) const
{
    pimpl_->get_locator_async(result, exception, ctx, opts);
}

void
adapter::get_locator_registry_async(
        functional::callback<locator_registry_prx>  result,
        functional::exception_callback              exception,
        context_type const&                         ctx,
        invocation_options const&                   opts) const
{
    pimpl_->get_locator_registry_async(result, exception, ctx, opts);
}

void
adapter::register_adapter_async(
        functional::void_callback       __result,
        functional::exception_callback  __exception,
        context_type const&             ctx,
        invocation_options const&       opts)
{
    pimpl_->register_adapter_async(__result, __exception, ctx, opts);
}

void
adapter::unregister_adapter_async(
        functional::void_callback       __result,
        functional::exception_callback  __exception,
        context_type const&             ctx,
        invocation_options const&       opts)
{
    pimpl_->unregister_adapter_async(__result, __exception, ctx, opts);
}

bool
adapter::is_active() const
{
    return pimpl_->is_active_;
}

identity const&
adapter::id() const
{
    return pimpl_->id_;
}

object_prx
adapter::adapter_proxy() const
{
    return pimpl_->adapter_proxy();
}

endpoint_list const&
adapter::configured_endpoints() const
{
    return pimpl_->options_.endpoints;
}

endpoint_list
adapter::published_endpoints() const
{
    return pimpl_->published_endpoints();
}

object_prx
adapter::create_proxy(identity const& id,
        ::std::string const& facet) const
{
    return pimpl_->create_proxy(id, facet);
}

object_prx
adapter::create_direct_proxy(identity const& id,
        ::std::string const& facet) const
{
    return pimpl_->create_direct_proxy(id, facet);
}

object_prx
adapter::create_indirect_proxy(identity const& id,
        ::std::string const& facet) const
{
    return pimpl_->create_indirect_proxy(id, facet);
}

object_prx
adapter::create_replicated_proxy(identity const& id,
        ::std::string const& facet) const
{
    return pimpl_->create_replicated_proxy(id, facet);
}

object_prx
adapter::add_object(object_ptr disp)
{
    return pimpl_->add_object(identity::random(), disp);
}

object_prx
adapter::add_object(identity const& id, object_ptr disp)
{
    return pimpl_->add_object(id, disp);
}

void
adapter::remove_object(identity const& id)
{
    pimpl_->remove_object(id);
}

void
adapter::add_default_servant(object_ptr disp)
{
    pimpl_->add_default_servant(DEFAULT_CATEGORY, disp);
}

void
adapter::remove_default_servant()
{
    pimpl_->remove_default_servant(DEFAULT_CATEGORY);
}

void
adapter::add_default_servant(::std::string const& category, object_ptr disp)
{
    pimpl_->add_default_servant(category, disp);
}

void
adapter::remove_default_servant(::std::string const& category)
{
    pimpl_->remove_default_servant(category);
}

void
adapter::add_object_locator(object_locator_ptr loc)
{
    pimpl_->add_locator(DEFAULT_CATEGORY, loc);
}

void
adapter::add_object_locator(::std::string const& category, object_locator_ptr loc)
{
    pimpl_->add_locator(category, loc);
}

object_ptr
adapter::find_object(identity const& id, ::std::string const& facet) const
{
    return pimpl_->find_object(id, facet);
}

bool
adapter::dispatch(detail::dispatch_request const& req, current const& curr)
{
    return pimpl_->dispatch(req, curr);
}

void
adapter::add_observer(connection_observer_ptr observer)
{
    pimpl_->add_observer(observer);
}

void
adapter::remove_observer(connection_observer_ptr observer)
{
    pimpl_->remove_observer(observer);
}

connection_observer_set
adapter::connection_observers() const
{
    return pimpl_->connection_observers();
}

void
adapter::connection_online(endpoint const& local, endpoint const& remote)
{
    pimpl_->connection_online(local, remote);
}

void
adapter::listen_connection_online(endpoint const& local)
{
    pimpl_->listen_connection_online(local);
}


void
adapter::connection_offline(endpoint const& ep)
{
    pimpl_->connection_offline(ep);
}

}  // namespace core
}  // namespace wire
