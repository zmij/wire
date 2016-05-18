/*
 * adapter_admin_impl.cpp
 *
 *  Created on: May 16, 2016
 *      Author: zmij
 */

#include <wire/core/adapter_admin_impl.hpp>
#include <wire/core/connector.hpp>
#include <wire/core/adapter.hpp>

namespace wire {
namespace core {

adapter_admin_impl::adapter_admin_impl(connector_ptr conn)
    : connector_{conn}
{
}

identity
adapter_admin_impl::get_id(current const& curr) const
{
    if (get_adapter(curr.operation.identity))
        return curr.operation.identity;
    throw no_adapter{};
}

object_prx
adapter_admin_impl::get_adapter_proxy(current const& curr) const
{
    if (auto ada = get_adapter(curr.operation.identity))
        return ada->adapter_proxy();
    throw no_adapter{};
}

adapter_state
adapter_admin_impl::get_state(current const& curr) const
{
    if (auto ada = get_adapter(curr.operation.identity))
        return ada->is_active() ? adapter_state::active : adapter_state::inactive;
    throw no_adapter{};
}

void
adapter_admin_impl::activate(functional::void_callback __resp,
        functional::exception_callback __exception,
        current const& curr)
{
    if (auto ada = get_adapter(curr.operation.identity)) {
        if (!ada->is_active()) {
            try {
                ada->activate();
                __resp();
            } catch (...) {
                __exception( ::std::current_exception() );
            }
        } else {
            __exception( ::std::make_exception_ptr(adapter_active{}) );
        }
        return;
    }
    __exception( ::std::make_exception_ptr(no_adapter{}) );
}

void
adapter_admin_impl::deactivate(functional::void_callback __resp,
        functional::exception_callback __exception,
        current const& curr)
{
    if (auto ada = get_adapter(curr.operation.identity)) {
        if (ada->is_active()) {
            try {
                ada->deactivate();
                __resp();
            } catch (...) {
                __exception( ::std::current_exception() );
            }
        } else {
            __exception( ::std::make_exception_ptr(adapter_inactive{}) );
        }
        return;
    }
    __exception( ::std::make_exception_ptr(no_adapter{}) );
}

adapter_ptr
adapter_admin_impl::get_adapter(identity const& id) const
{
    auto cnctr = get_connector();
    if (cnctr) {
        return cnctr->find_adapter(id);
    }
    return adapter_ptr{};
}

} /* namespace core */
} /* namespace wire */
