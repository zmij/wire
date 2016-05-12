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

#include <wire/core/detail/configuration_options.hpp>

#include <unordered_map>
#include <mutex>

namespace wire {
namespace core {

namespace {

::std::string const DEFAULT_CATEGORY{};

}  /* namespace  */

struct adapter::impl {
    using connections       = ::std::unordered_map< endpoint, connection >;
    using active_objects    = ::std::unordered_map< identity, object_ptr >;
    using default_servants  = ::std::unordered_map< ::std::string, object_ptr >;
    using object_locators   = ::std::unordered_map< ::std::string, object_locator_ptr >;
    using mutex_type        = ::std::mutex;
    using lock_guard        = ::std::lock_guard<mutex_type>;

    connector_weak_ptr          connector_;
    asio_config::io_service_ptr io_service_;
    identity                    id_;

    detail::adapter_options     options_;

    connections                 connections_;
    active_objects              active_objects_;
    default_servants            default_servants_;
    object_locators             locators_;

    adapter_weak_ptr            owner_;

    mutex_type                  mtx_;

    impl(connector_ptr c, identity const& id, detail::adapter_options const& options)
        : connector_{c}, io_service_{c->io_service()}, id_{id}, options_{options}
    {
    }

    void
    activate()
    {
        lock_guard lock{mtx_};
        adapter_ptr adp = owner_.lock();
        if (adp) {
            if (options_.endpoints.empty()) {
                options_.endpoints.push_back(endpoint::tcp("0.0.0.0", 0));
            }
            for (auto const& ep : options_.endpoints) {
                connections_.emplace(ep, connection{server_side{}, adp, ep });
            }
        } else {
            throw ::std::runtime_error("Adapter owning implementation was destroyed");
        }
    }

    bool
    is_local_endpoint(endpoint const& ep)
    {
        return connections_.count(ep);
    }

    void
    deactivate()
    {
        lock_guard lock{mtx_};
        for (auto& c : connections_) {
            c.second.close();
        }
        connections_.clear();
    }

    void
    connection_online(endpoint const& local, endpoint const& remote)
    {
        ::std::cerr << "Connection " << local << " -> " << remote << " is online\n";
    }
    void
    listen_connection_online(endpoint const& ep)
    {
        auto cr = connector_.lock();
        if (!cr)
            throw errors::runtime_error{ "Connector is dead" };
        cr->adapter_online(owner_.lock(), ep);
    }
    void
    connection_offline(endpoint const& ep)
    {
        ::std::cerr << "Connection " << ep << " is offline\n";
    }

    endpoint_list
    active_endpoints()
    {
        endpoint_list endpoints;
        for (auto const& cn : connections_) {
            endpoints.push_back(cn.second.local_endpoint());
        }
        return endpoints;
    }

    object_prx
    add_object(identity const& id, object_ptr disp)
    {
        lock_guard lock{mtx_};
        active_objects_.insert(::std::make_pair(id, disp));
        return ::std::make_shared< object_proxy >(
            reference::create_reference(
                connector_.lock(), { id, {}, {}, active_endpoints() }));
    }

    void
    add_default_servant(::std::string const& category, object_ptr disp)
    {
        lock_guard lock{mtx_};
        default_servants_.emplace(category, disp);
    }

    void
    add_locator(::std::string const& category, object_locator_ptr loc)
    {
        lock_guard lock{mtx_};
        locators_.emplace(category, loc);
    }

    object_ptr
    find_object(identity const& id, ::std::string const& facet)
    {
        lock_guard lock{mtx_};
        auto o = active_objects_.find(id);
        if (o != active_objects_.end()) {
            return o->second;
        }
        auto d = default_servants_.find(id.category);
        if (d != default_servants_.end()) {
            return d->second;
        }
        if (!id.category.empty()) {
            d = default_servants_.find(DEFAULT_CATEGORY);
            if (d != default_servants_.end()) {
                return d->second;
            }
        }
        auto loc = locators_.find(id.category);
        if (loc != locators_.end()) {
            auto obj = loc->second->find_object(owner_.lock(), id, facet);
            if (obj)
                return obj;
        }
        if (!id.category.empty()) {
            loc = locators_.find(DEFAULT_CATEGORY);
            if (loc != locators_.end()) {
                return loc->second->find_object(owner_.lock(), id, facet);
            }
        }

        return object_ptr{};
    }
};

adapter_ptr
adapter::create_adapter(connector_ptr c, identity const& id,
        detail::adapter_options const& options)
{
    adapter_ptr a(new adapter{c, id, options});
    a->pimpl_->owner_ = a;
    return a;
}

adapter::adapter(connector_ptr c, identity const& id,
        detail::adapter_options const& options)
    : pimpl_( ::std::make_shared<impl>(c, id, options) )
{
}

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

void
adapter::activate()
{
    pimpl_->activate();
}

identity const&
adapter::id() const
{
    return pimpl_->id_;
}

endpoint_list const&
adapter::configured_endpoints() const
{
    return pimpl_->options_.endpoints;
}

endpoint_list
adapter::active_endpoints() const
{
    return pimpl_->active_endpoints();
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

}

void
adapter::add_object_locator(::std::string const& category, object_locator_ptr loc)
{

}

object_ptr
adapter::find_object(identity const& id, ::std::string const& facet) const
{
    return pimpl_->find_object(id, facet);
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
