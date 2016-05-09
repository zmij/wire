/*
 * adapter.cpp
 *
 *  Created on: 7 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/core/adapter.hpp>
#include <wire/core/connection.hpp>
#include <wire/core/connector.hpp>

#include <wire/core/detail/configuration_options.hpp>

#include <unordered_map>

namespace wire {
namespace core {

struct adapter::impl {
    using connections       = ::std::unordered_map< endpoint, connection >;
    using active_objects    = ::std::unordered_map< identity, dispatcher_ptr >;
    using default_servants  = ::std::unordered_map< ::std::string, dispatcher_ptr >;

    connector_weak_ptr          connector_;
    asio_config::io_service_ptr io_service_;
    identity                    id_;

    detail::adapter_options     options_;

    connections                 connections_;
    active_objects              active_objects_;
    default_servants            default_servants_;

    adapter_weak_ptr            owner_;

    impl(connector_ptr c, identity const& id, detail::adapter_options const& options)
        : connector_{c}, io_service_{c->io_service()}, id_{id}, options_{options}
    {
    }

    void
    activate()
    {
        adapter_ptr adp = owner_.lock();
        if (adp) {
            if (options_.endpoints.empty()) {
                options_.endpoints.push_back(endpoint::tcp("0.0.0.0", 0));
            }
            for (auto const& ep : options_.endpoints) {
                connections_.emplace(ep, ::std::move(connection{ adp, ep }));
            }
        } else {
            throw ::std::runtime_error("Adapter owning implementation was destroyed");
        }
    }

    void
    deactivate()
    {
        for (auto& c : connections_) {
            c.second.close();
        }
        connections_.clear();
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

    void
    add_object(identity const& id, dispatcher_ptr disp)
    {
        active_objects_.insert(::std::make_pair(id, disp));
    }

    void
    add_default_servant(::std::string const& category, dispatcher_ptr disp)
    {
        default_servants_.insert(::std::make_pair(category, disp));
    }

    dispatcher_ptr
    find_object(identity const& id)
    {
        auto o = active_objects_.find(id);
        if (o != active_objects_.end()) {
            return o->second;
        }
        auto d = default_servants_.find(id.category);
        if (d != default_servants_.end()) {
            return d->second;
        }
        d = default_servants_.find("");
        if (d != default_servants_.end()) {
            return d->second;
        }
        return dispatcher_ptr{};
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
adapter::name() const
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

void
adapter::add_object(dispatcher_ptr disp)
{
    pimpl_->add_object(identity::random(), disp);
}

void
adapter::add_object(identity const& id, dispatcher_ptr disp)
{
    pimpl_->add_object(id, disp);
}

void
adapter::add_default_servant(dispatcher_ptr disp)
{
    pimpl_->add_default_servant("", disp);
}

void
adapter::add_default_servant(::std::string const& category, dispatcher_ptr disp)
{
    pimpl_->add_default_servant(category, disp);
}

dispatcher_ptr
adapter::find_object(identity const& id) const
{
    return pimpl_->find_object(id);
}

}  // namespace core
}  // namespace wire
