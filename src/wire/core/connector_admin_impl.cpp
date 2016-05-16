/*
 * connector_admin_impl.cpp
 *
 *  Created on: May 16, 2016
 *      Author: zmij
 */

#include <wire/core/connector_admin_impl.hpp>

namespace wire {
namespace core {

connector_admin_impl::connector_admin_impl()
{
}

identity_seq
connector_admin_impl::get_adapter_ids(current const& curr)
{
    return identity_seq{};
}
void
connector_admin_impl::get_adapter_state(identity const& adapter_id,
        get_adapter_state_return_callback __resp,
        functional::exception_callback __exception,
        current const& curr)
{
    __resp(adapter_state::inactive);
}

void
connector_admin_impl::activate(identity const& adapter_id,
        functional::void_callback __resp,
        functional::exception_callback __exception,
        current const& curr)
{
    __resp();
}

void
connector_admin_impl::deactivate(identity const& adapter_id,
        functional::void_callback __resp,
        functional::exception_callback __exception,
        current const& curr)
{
    __resp();
}

void
connector_admin_impl::get_adapter_admin(identity const& adapter_id,
        get_adapter_admin_return_callback __resp,
        functional::exception_callback __exception,
        current const& curr)
{
    __resp(adapter_admin_prx{});
}

} /* namespace core */
} /* namespace wire */
