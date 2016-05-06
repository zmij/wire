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
    typedef ::std::unordered_map< endpoint, connection > connections;
    typedef ::std::unordered_map< identity, dispatcher_ptr > active_objects;
    typedef ::std::unordered_map< ::std::string, dispatcher_ptr > default_servants;

    asio_config::io_service_ptr io_service_;
    ::std::string                name_;

    detail::adapter_options        options_;

    connections                    connections_;
    active_objects                active_objects_;
    default_servants            default_servants_;

    adapter_weak_ptr            owner_;

    impl(connector_ptr c, ::std::string const& name, detail::adapter_options const& options)
        : io_service_(c->io_service()), name_(name), options_(options)
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
                connections_.emplace(ep, std::move(connection{ adp, ep }));
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
        ;
        return ::std::move(endpoints);
    }

    void
    add_object(identity const& id, dispatcher_ptr disp)
    {
        active_objects_.insert(std::make_pair(id, disp));
    }

    void
    add_default_servant(std::string const& category, dispatcher_ptr disp)
    {
        default_servants_.insert(std::make_pair(category, disp));
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
adapter::create_adapter(connector_ptr c, ::std::string const& name,
        detail::adapter_options const& options)
{
    adapter_ptr a(new adapter{c, name, options});
    a->pimpl_->owner_ = a;
    return a;
}

adapter::adapter(connector_ptr c, ::std::string const& name,
        detail::adapter_options const& options)
    : pimpl_( ::std::make_shared<impl>(c, name, options) )
{
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

::std::string const&
adapter::name() const
{
    return pimpl_->name_;
}

endpoint_list const&
adapter::endpoints() const
{
    return pimpl_->options_.endpoints;
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
