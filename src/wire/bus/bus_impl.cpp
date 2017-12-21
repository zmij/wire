/*
 * bus_impl.cpp
 *
 *  Created on: 8 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/bus/bus_impl.hpp>

#include <wire/errors/not_found.hpp>
#include <wire/core/adapter.hpp>

namespace wire {
namespace svc {

bus_impl::bus_impl(core::adapter_ptr adapter, publisher_ptr pub)
    : adapter_{adapter}, publisher_{pub}
{
}

bool
bus_impl::wire_is_a(std::string const& type, core::current const& curr) const
{
    if (!publisher_->bus_exists(curr.operation.target.identity))
        throw errors::no_object{
            curr.operation.target.identity,
            curr.operation.target.facet,
            "wire_is_a"
        };
    return bus::wire_is_a(type, curr);
}

void
bus_impl::wire_ping(core::current const& curr) const
{
    if (!publisher_->bus_exists(curr.operation.target.identity))
        throw errors::no_object{
            curr.operation.target.identity,
            curr.operation.target.facet,
            "wire_ping"
        };
}

::std::string const&
 bus_impl::wire_type(core::current const& curr) const
{
    if (!publisher_->bus_exists(curr.operation.target.identity))
        throw errors::no_object{
            curr.operation.target.identity,
            curr.operation.target.facet,
            "wire_type"
        };
    return bus::wire_type(curr);
}

core::object::type_list const&
bus_impl::wire_types(core::current const& curr) const
{
    if (!publisher_->bus_exists(curr.operation.target.identity))
        throw errors::no_object{
            curr.operation.target.identity,
            curr.operation.target.facet,
            "wire_type"
        };
    return bus::wire_types(curr);
}

void
bus_impl::get_publisher(get_publisher_return_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const& curr)
{
    try {
        if (publisher_->bus_exists(curr.operation.target.identity)) {
            auto adapter = get_adapter();
            __resp(adapter->create_direct_proxy(curr.operation.target.identity));
        } else {
            throw errors::no_object{
                curr.operation.target.identity,
                curr.operation.target.facet,
                "get_publisher"
            };
        }
    } catch (...) {
        __exception(::std::current_exception());
    }
}

void
bus_impl::subscribe(::wire::core::object_prx subscriber,
        ::wire::core::functional::void_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const& curr)
{
    try {
        publisher_->add_subscriber( curr.operation.target.identity, subscriber, curr );
        __resp();
    } catch (...) {
        __exception(::std::current_exception());
    }
}

void
bus_impl::unsubscribe(::wire::core::object_prx subscriber,
        ::wire::core::functional::void_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const& curr)
{
    try {
        publisher_->remove_subscriber( curr.operation.target.identity, subscriber, curr );
        __resp();
    } catch (...) {
        __exception(::std::current_exception());
    }
}

//----------------------------------------------------------------------------
registry_impl::registry_impl(core::adapter_ptr adapter, publisher_ptr pub)
    : adapter_{adapter}, publisher_{pub}
{
}

void
registry_impl::create_bus(::wire::core::identity&& id,
        create_bus_return_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const& curr)
{
    try {
        publisher_->add_bus(id);
        auto adapter = get_adapter();
        auto prx = adapter->create_direct_proxy(id);
        __resp(core::unchecked_cast< bus::bus_proxy >(prx));
    } catch (...) {
        __exception(::std::current_exception());
    }
}

void
registry_impl::get_bus(::wire::core::identity&& id,
        get_bus_return_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const& curr)
{
    try {
        if (publisher_->bus_exists(id)) {
            auto adapter = get_adapter();
            auto prx = adapter->create_direct_proxy(id);
            __resp(core::unchecked_cast< bus::bus_proxy >(prx));
        } else {
            throw bus::no_bus{id};
        }
    } catch (...) {
        __exception(::std::current_exception());
    }
}

void
registry_impl::get_or_create_bus(::wire::core::identity&& id,
        get_or_create_bus_return_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const& curr)
{
    try {
        if (publisher_->bus_exists(id)) {
            auto adapter = get_adapter();
            auto prx = adapter->create_direct_proxy(id);
            __resp(core::unchecked_cast< bus::bus_proxy >(prx));
        } else {
            publisher_->add_bus(id);
            auto adapter = get_adapter();
            auto prx = adapter->create_direct_proxy(id);
            __resp(core::unchecked_cast< bus::bus_proxy >(prx));
        }
    } catch (...) {
        __exception(::std::current_exception());
    }
}

void
registry_impl::remove_bus(::wire::core::identity&& id,
        ::wire::core::functional::void_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const& curr)
{
    try {
        publisher_->remove_bus(id);
        __resp();
    } catch (...) {
        __exception(::std::current_exception());
    }
}

}  /* namespace svc */
}  /* namespace wire */
