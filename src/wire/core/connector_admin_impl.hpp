/*
 * connector_admin_impl.hpp
 *
 *  Created on: 18 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_CORE_CONNECTOR_ADMIN_IMPL_HPP_
#define WIRE_CORE_CONNECTOR_ADMIN_IMPL_HPP_

#include <wire/core/connector_admin.hpp>
#include <wire/core/connector_fwd.hpp>
#include <wire/core/adapter_fwd.hpp>

namespace wire {
namespace core {

class connector_admin_impl: public connector_admin {
public:
    connector_admin_impl(connector_ptr cnctr, adapter_ptr adm);
    virtual ~connector_admin_impl() {}

    //@{
    /** @name Interface implementation */
    identity_seq
    get_adapter_ids(current const& = no_current) const override;
    void
    get_adapter_state(identity const& adapter_id,
            get_adapter_state_return_callback __resp,
            functional::exception_callback __exception,
            current const& = no_current) const override;
    void
    activate(identity const& adapter_id,
            functional::void_callback __resp,
            functional::exception_callback __exception,
            current const& = no_current) override;
    void
    deactivate(identity const& adapter_id,
            functional::void_callback __resp,
            functional::exception_callback __exception,
            current const& = no_current) override;
    void
    get_adapter_admin(identity const& adapter_id,
            get_adapter_admin_return_callback __resp,
            functional::exception_callback __exception,
            current const& = no_current) const override;
    //@}
private:
    connector_ptr
    get_connector() const
    { return connector_.lock(); }
    adapter_ptr
    get_adapter() const
    { return adapter_.lock(); }
    adapter_ptr
    get_adapter(identity const& id) const;
private:
    connector_weak_ptr connector_;
    adapter_weak_ptr adapter_;
};

} /* namespace core */
} /* namespace wire */

#endif /* WIRE_CORE_CONNECTOR_ADMIN_IMPL_HPP_ */
