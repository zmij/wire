/*
 * connector_admin_impl.hpp
 *
 *  Created on: May 16, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_CONNECTOR_ADMIN_IMPL_HPP_
#define WIRE_CORE_CONNECTOR_ADMIN_IMPL_HPP_

#include <wire/core/connector_admin.hpp>
#include <wire/core/connector_fwd.hpp>

namespace wire {
namespace core {

class connector_admin_impl: public connector_admin {
public:
    connector_admin_impl();
    virtual ~connector_admin_impl() {}

    //@{
    identity_seq
    get_adapter_ids(current const& = no_current) override;
    void
    get_adapter_state(identity const& adapter_id,
            get_adapter_state_return_callback __resp,
            functional::exception_callback __exception,
            current const& = no_current) override;
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
            current const& = no_current) override;
    //@}

private:
    connector_weak_ptr  connector_;
};

} /* namespace core */
} /* namespace wire */

#endif /* WIRE_CORE_CONNECTOR_ADMIN_IMPL_HPP_ */
