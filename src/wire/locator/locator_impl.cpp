/*
 * locator_impl.cpp
 *
 *  Created on: Sep 27, 2016
 *      Author: zmij
 */

#include <wire/locator/locator_impl.hpp>
#include <wire/util/make_unique.hpp>
#include <wire/errors/not_found.hpp>

#include <mutex>
#include <unordered_map>

namespace wire {
namespace svc {

struct locator::impl {
    using mutex_type        = ::std::mutex;
    using lock_guard        = ::std::lock_guard<mutex_type>;
    using proxy_map         = ::std::unordered_map< core::identity, core::object_prx >;

    core::locator_registry_prx  reg_;

    mutex_type                  object_mutex_;
    proxy_map                   objects_;

    mutex_type                  adapter_mutex_;
    proxy_map                   adapters_;

    core::object_prx
    find_object(core::identity const& id)
    {
        lock_guard lock{object_mutex_};
        auto f = objects_.find(id);
        if (f != objects_.end()) {
            return f->second;
        }
        throw errors::no_object{id, "", ""};
    }

    void
    add_well_known_object(core::object_prx obj)
    {
        if (obj) {
            auto id = obj->wire_identity();
            lock_guard lock{object_mutex_};
            auto f = objects_.find(id);
            if (f == objects_.end()) {
                objects_.emplace(id, obj);
            } else {
                if (*f->second != *obj) {
                    bool alive = true;
                    try {
                        f->second->wire_ping();
                        // Object is alive, throw exception
                    } catch (...) {
                        // Object is dead, safe to replace
                        alive = false;
                    }
                    if (alive) {
                        // TODO Throw an exception
                        throw core::well_known_object_exists{};
                    } else {
                        f->second = obj;
                    }
                }
            }
        }
    }

    core::object_prx
    find_adapter(core::identity const& id)
    {
        lock_guard lock{adapter_mutex_};
        auto f = adapters_.find(id);
        if (f != adapters_.end()) {
            return f->second;
        }
        throw core::no_adapter{id};
    }

    void
    add_adapter(core::object_prx adapter)
    {
        auto id = adapter->wire_identity();
        lock_guard lock{adapter_mutex_};
        auto f = adapters_.find(id);
        if (f == adapters_.end()) {
            adapters_.emplace(id, adapter);
        } else {
            throw core::adapter_exists{id};
        }
    }
    void
    add_replicated_adapter(core::object_prx adapter)
    {
        auto id = adapter->wire_identity();
        lock_guard lock{adapter_mutex_};
        auto f = adapters_.find(id);
        if (f == adapters_.end()) {
            adapters_.emplace(id, adapter);
        } else {
            auto data = f->second->wire_get_reference()->data();
            auto const& new_data = adapter->wire_get_reference()->data();
            core::endpoint_set eps{data.endpoints.begin(), data.endpoints.end()};

            ::std::copy(
                new_data.endpoints.begin(), new_data.endpoints.end(),
                ::std::inserter(eps, eps.begin())
            );

            data.endpoints.clear();
            data.endpoints.reserve(eps.size());
            ::std::copy(eps.begin(), eps.end(), ::std::back_inserter(data.endpoints));

            auto ref = core::reference::create_reference(reg_->wire_get_connector(), data);
            f->second = ::std::make_shared< core::object_proxy >( ref );
        }
    }
    void
    remove_adapter(core::object_prx adapter)
    {
        auto id = adapter->wire_identity();
        lock_guard lock{adapter_mutex_};
        auto f = adapters_.find(id);
        if (f == adapters_.end()) {
            throw core::no_adapter{id};
        } else {
            auto data = f->second->wire_get_reference()->data();
            auto const& new_data = adapter->wire_get_reference()->data();
            core::endpoint_set old_eps{data.endpoints.begin(), data.endpoints.end()};
            core::endpoint_set remove_eps{new_data.endpoints.begin(), new_data.endpoints.end()};

            data.endpoints.clear();
            ::std::set_difference(
                old_eps.begin(), old_eps.end(),
                remove_eps.begin(), remove_eps.end(),
                ::std::back_inserter(data.endpoints));
            if (data.endpoints.empty()) {
                adapters_.erase(f);
            } else {
                auto ref = core::reference::create_reference(reg_->wire_get_connector(), data);
                f->second = ::std::make_shared< core::object_proxy >( ref );
            }
        }
    }
};

locator::locator(core::locator_registry_prx registry)
    : pimpl_{util::make_unique<impl>()}
{
}

locator::~locator() = default;

void
locator::find_object(::wire::core::identity const& id,
            find_object_return_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const&) const
{
    try {
        __resp(pimpl_->find_object(id));
    } catch (...) {
        __exception(::std::current_exception());
    }
}

void
locator::find_adapter(::wire::core::identity const& id,
        find_adapter_return_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const&) const
{
    try {
        __resp(pimpl_->find_adapter(id));
    } catch (...) {
        __exception(::std::current_exception());
    }
}

void
locator::get_registry(get_registry_return_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const&) const
{
    try {
        __resp(pimpl_->reg_);
    } catch (...) {
        __exception(::std::current_exception());
    }
}

void
locator::add_well_known_object(core::object_prx obj)
{
    pimpl_->add_well_known_object(obj);
}

void
locator::add_adapter(core::object_prx adapter)
{
    pimpl_->add_adapter(adapter);
}

void
locator::add_replicated_adapter(core::object_prx adapter)
{
    pimpl_->add_adapter(adapter);
}

void
locator::remove_adapter(core::object_prx adapter)
{
    pimpl_->add_adapter(adapter);
}

//----------------------------------------------------------------------------
//  Registry interface implementation
//----------------------------------------------------------------------------
locator_registry::locator_registry(locator_ptr loc)
    : loc_{loc}
{
}

void
locator_registry::add_well_known_object(::wire::core::object_prx obj,
        ::wire::core::functional::void_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const&)
{
    try {
        loc_->add_well_known_object(obj);
        __resp();
    } catch (...) {
        __exception(::std::current_exception());
    }
}

void
locator_registry::add_adapter(::wire::core::object_prx adapter,
        ::wire::core::functional::void_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const&)
{
    try {
        loc_->add_adapter(adapter);
        __resp();
    } catch (...) {
        __exception(::std::current_exception());
    }
}

void
locator_registry::add_replicated_adapter(::wire::core::object_prx adapter,
        ::wire::core::functional::void_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const&)
{
    try {
        loc_->add_replicated_adapter(adapter);
        __resp();
    } catch (...) {
        __exception(::std::current_exception());
    }
}

void
locator_registry::remove_adapter(::wire::core::object_prx adapter,
        ::wire::core::functional::void_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const&)
{
    try {
        loc_->remove_adapter(adapter);
        __resp();
    } catch (...) {
        __exception(::std::current_exception());
    }
}

}  /* namespace service */
}  /* namespace wire */

