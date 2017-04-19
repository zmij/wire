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

#include <wire/core/detail/configuration_options.hpp>
#include <wire/errors/not_found.hpp>

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

struct adapter::impl {
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

    connector_weak_ptr          connector_;
    asio_config::io_service_ptr io_service_;
    identity                    id_;

    detail::adapter_options     options_;
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

    impl(connector_ptr c, identity const& id, detail::adapter_options const& options,
            connection_observer_set const& observers)
        : connector_{c}, io_service_{c->io_service()},
          id_{id}, options_{options}, is_active_{false}, registered_{false},
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
            if (!postpone_reg)
                register_adapter();
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
    register_adapter()
    {
        auto cnctr = connector_.lock();
        if (!cnctr)
            // FIXME Correct exception
            throw errors::connector_destroyed{"Connector was already destroyed"};
        if (options_.registered || is_replicated()) {
            locator_registry_prx reg;
            try {
                reg = cnctr->get_locator_registry(options_.locator_ref);
            } catch (...) {
                #if DEBUG_OUTPUT >= 1
                ::std::cerr <<::getpid() << "  Failed to get locator registry on exception\n";
                #endif
            }
            if (reg) {
                auto prx = adapter_proxy();
                if (is_replicated()) {
                    #if DEBUG_OUTPUT >= 2
                    ::std::cerr <<::getpid() << " Try to register replicated adapter " << *prx << " at locator\n";
                    #endif
                    reg->add_replicated_adapter(prx);
                } else {
                    #if DEBUG_OUTPUT >= 2
                    ::std::cerr <<::getpid() << " Try to register adapter " << *prx << " at locator\n";
                    #endif
                    reg->add_adapter(prx);
                }
                registered_ = true;
            } else if (is_replicated()) {
                throw errors::runtime_error{
                        "wire.locator not configured for a replicated adapter"};
            }

        }
    }

    bool
    is_local_endpoint(endpoint const& ep)
    {
        connections::const_accessor acc;
        return connections_.find(acc, ep);
    }

    void
    deactivate()
    {
        lock_guard lock{mtx_};
        if (registered_) {
            auto cnctr = connector_.lock();
            if (cnctr) {
                try {
                    auto reg = cnctr->get_locator_registry();
                    if (reg) {
                        auto prx = adapter_proxy();
                        reg->remove_adapter(prx);
                    }
                } catch ( ::std::exception const& e) {
                    #if DEBUG_OUTPUT >= 1
                    ::std::cerr <<::getpid() << " Error wnen unregistering adapter " << id_<< ": "
                            << e.what() << "\n";
                    #endif
                } catch (...) {
                    #if DEBUG_OUTPUT >= 1
                    ::std::cerr <<::getpid() << " Unexpected error when unregistering adapter "
                            << id_ << "\n";
                    #endif
                }
            }
            registered_ = false;
        }
        for (auto& c : connections_) {
            c.second->close();
        }
        connections_.clear();
        published_endpoints_.clear();
        is_active_ = false;
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
        #ifdef DEBUG_OUTPUT
        ::std::cerr <<::getpid() << " Connection " << local << " -> " << remote << " is online\n";
        #endif
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
        #ifdef DEBUG_OUTPUT
        ::std::cerr <<::getpid() << " Connection " << ep << " is offline\n";
        #endif
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

void
adapter::activate(bool postpone_reg)
{
    pimpl_->activate(postpone_reg);
}

void
adapter::register_adapter()
{
    pimpl_->register_adapter();
}

void
adapter::deactivate()
{
    pimpl_->deactivate();
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
adapter::add_default_servant(object_ptr disp)
{
    pimpl_->add_default_servant(DEFAULT_CATEGORY, disp);
}

void
adapter::add_default_servant(::std::string const& category, object_ptr disp)
{
    pimpl_->add_default_servant(category, disp);
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
