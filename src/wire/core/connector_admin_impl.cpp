/*
 * connector_admin_impl.cpp
 *
 *  Created on: 18 мая 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/core/connector_admin_impl.hpp>
#include <wire/core/connector.hpp>
#include <wire/core/adapter.hpp>

namespace wire {
namespace core {

connector_admin_impl::connector_admin_impl(connector_ptr cnctr, adapter_ptr adm)
    : object(), connector_{cnctr}, adapter_{adm}
{
}

identity_seq
connector_admin_impl::get_adapter_ids(current const&) const
{
    auto cnctr = get_connector();
    if (cnctr)
        return cnctr->adapter_ids();
    throw errors::connector_destroyed{ "Connector is shut down" };
}
void
connector_admin_impl::get_adapter_state(identity const& adapter_id,
        get_adapter_state_return_callback __resp,
        functional::exception_callback __exception,
        current const&) const
{
    if (auto ada = get_adapter(adapter_id)) {
        __resp(ada->is_active() ? adapter_state::active : adapter_state::inactive);
        return;
    }
    __exception( ::std::make_exception_ptr(no_adapter{}) );
}

void
connector_admin_impl::activate(identity const& adapter_id,
        functional::void_callback __resp,
        functional::exception_callback __exception,
        current const&)
{
    if (auto ada = get_adapter(adapter_id)) {
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
connector_admin_impl::deactivate(identity const& adapter_id,
        functional::void_callback __resp,
        functional::exception_callback __exception,
        current const&)
{
    if (auto ada = get_adapter(adapter_id)) {
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

void
connector_admin_impl::get_adapter_admin(identity const& adapter_id,
        get_adapter_admin_return_callback __resp,
        functional::exception_callback __exception,
        current const&) const
{
    if (auto ada = get_adapter(adapter_id)) {
        auto prx = get_adapter()->create_proxy(adapter_id);
        auto adm_prx = unchecked_cast< adapter_admin_proxy >(prx);
        __resp(adm_prx);
        return;
    }
    __exception( ::std::make_exception_ptr(no_adapter{}) );
}

adapter_ptr
connector_admin_impl::get_adapter(identity const& id) const
{
    auto cnctr = get_connector();
    if (cnctr) {
        return cnctr->find_adapter(id);
    }
    return adapter_ptr{};
}

} /* namespace core */
} /* namespace wire */
