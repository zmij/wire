/*
 * adapter_admin_impl.cpp
 *
 *  Created on: May 16, 2016
 *      Author: zmij
 */

#include <wire/core/adapter_admin_impl.hpp>

namespace wire {
namespace core {

adapter_admin_impl::adapter_admin_impl(connector_ptr conn)
    : connector_{conn}
{
}

identity
adapter_admin_impl::get_id(current const& curr) const
{
    return curr.operation.identity;
}

object_prx
adapter_admin_impl::get_adapter_proxy(current const& curr) const
{
    return object_prx{};
}

adapter_state
adapter_admin_impl::get_state(current const& curr) const
{
    return adapter_state::inactive;
}

void
adapter_admin_impl::activate(functional::void_callback __resp,
        functional::exception_callback __exception,
        current const& curr)
{
    __resp();
}

void
adapter_admin_impl::deactivate(functional::void_callback __resp,
        functional::exception_callback __exception,
        current const& curr)
{
    __resp();
}


} /* namespace core */
} /* namespace wire */
