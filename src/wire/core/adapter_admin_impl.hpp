/*
 * adapter_admin_impl.hpp
 *
 *  Created on: May 16, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_ADAPTER_ADMIN_IMPL_HPP_
#define WIRE_CORE_ADAPTER_ADMIN_IMPL_HPP_

#include <wire/core/adapter_admin.hpp>
#include <wire/core/connector_fwd.hpp>

namespace wire {
namespace core {

class adapter_admin_impl: public adapter_admin {
public:
    adapter_admin_impl(connector_ptr conn);
    virtual ~adapter_admin_impl() {}
    //@{
    /** @name Interface implementation */
    identity
    get_id(current const& = no_current) const override;
    object_prx
    get_adapter_proxy(current const& = no_current) const override;
    adapter_state
    get_state(current const& = no_current) const override;
    void
    activate(functional::void_callback __resp,
            functional::exception_callback __exception,
            current const& = no_current) override;
    void
    deactivate(functional::void_callback __resp,
            functional::exception_callback __exception,
            current const& = no_current) override;
    //@}
private:
    connector_weak_ptr  connector_;
};

} /* namespace core */
} /* namespace wire */

#endif /* WIRE_CORE_ADAPTER_ADMIN_IMPL_HPP_ */
