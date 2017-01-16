/*
 * locator_impl.hpp
 *
 *  Created on: Sep 26, 2016
 *      Author: zmij
 */

#ifndef WIRE_LOCATOR_LOCATOR_IMPL_HPP_
#define WIRE_LOCATOR_LOCATOR_IMPL_HPP_

#include <wire/core/locator.hpp>

namespace wire {
namespace svc {

class locator : public core::locator {
public:
    locator(core::locator_registry_prx registry);
    ~locator();

    //@{
    /** @name Wire interface implementation */
    void
    find_object(::wire::core::identity const& id,
            find_object_return_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) const override;
    void
    find_adapter(::wire::core::identity const& id,
            find_adapter_return_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) const override;
    void
    get_registry(get_registry_return_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) const override;
    //@}

    //@{
    /** @name Actual registry implementation */
    void
    add_well_known_object(::wire::core::object_prx object,
            ::wire::core::functional::void_callback __resp,
            ::wire::core::functional::exception_callback __exception);
    void
    add_well_known_objects(::wire::core::object_seq const& objs,
            ::wire::core::functional::void_callback __resp,
            ::wire::core::functional::exception_callback __exception);
    void
    add_well_known_object(::wire::core::object_prx object);
    void
    remove_well_known_object(::wire::core::object_prx object);
    void
    add_adapter(::wire::core::object_prx adapter);
    void
    add_replicated_adapter(::wire::core::object_prx adapter);
    void
    remove_adapter(::wire::core::object_prx adapter);
    //@}
private:
    struct impl;
    using pimpl = ::std::unique_ptr<impl>;
    pimpl pimpl_;
};

using locator_ptr = ::std::shared_ptr<locator>;

class locator_registry : public core::locator_registry {
public:
    locator_registry( locator_ptr loc );

    //@{
    /** @name Wire interface implementation */
    void
    add_well_known_object(::wire::core::object_prx obj,
            ::wire::core::functional::void_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) override;

    void
    add_well_known_objects(::wire::core::object_seq const& objs,
            ::wire::core::functional::void_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) override;

    void
    remove_well_known_object(::wire::core::object_prx obj,
            ::wire::core::functional::void_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) override;

    void
    add_adapter(::wire::core::object_prx adapter,
            ::wire::core::functional::void_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) override;
    void
    add_replicated_adapter(::wire::core::object_prx adapter,
            ::wire::core::functional::void_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) override;
    void
    remove_adapter(::wire::core::object_prx adapter,
            ::wire::core::functional::void_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) override;
    //@}
private:
    locator_ptr loc_;
};

}  /* namespace service */
}  /* namespace wire */

#endif /* WIRE_LOCATOR_LOCATOR_IMPL_HPP_ */
