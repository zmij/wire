/*
 * bus_impl.hpp
 *
 *  Created on: 8 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_BUS_BUS_IMPL_HPP_
#define WIRE_BUS_BUS_IMPL_HPP_

#include <wire/bus/bus.hpp>
#include <wire/bus/publisher.hpp>
#include <wire/core/adapter_fwd.hpp>

namespace wire {
namespace svc {

class bus_impl : public bus::bus {
public:
    bus_impl(core::adapter_ptr publisher_adapter, publisher_ptr pub);
    virtual ~bus_impl() = default;

    //@{
    /** @name Wire object functions
     *  Functions are overloaded to throw no_object no bus exists.
     */
    bool
    wire_is_a(std::string const&, core::current const& = core::no_current) const override;
    void
    wire_ping(core::current const& = core::no_current) const override;
    ::std::string const&
    wire_type(core::current const& = core::no_current) const override;
    type_list const&
    wire_types(core::current const& = core::no_current) const override;
    //@}

    void
    get_publisher(get_publisher_return_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) override;
    void
    subscribe(::wire::core::object_prx subscriber,
            ::wire::core::functional::void_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) override;
    void
    unsubscribe(::wire::core::object_prx subscriber,
            ::wire::core::functional::void_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) override;
private:
    core::adapter_ptr
    get_adapter() const
    {
        // TODO Throw on adapter destroyed
        return adapter_.lock();
    }
    core::adapter_weak_ptr  adapter_;
    publisher_ptr           publisher_;
};

class registry_impl : public bus::bus_registry {
public:
    registry_impl(core::adapter_ptr adapter, publisher_ptr pub);
    virtual ~registry_impl() = default;

    void
    create_bus(::wire::core::identity const& id,
            create_bus_return_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) override;
    void
    get_bus(::wire::core::identity const& id,
            get_bus_return_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) override;
    void
    remove_bus(::wire::core::identity const& id,
            ::wire::core::functional::void_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) override;

private:
    core::adapter_ptr
    get_adapter() const
    {
        // TODO Throw on adapter destroyed
        return adapter_.lock();
    }
    core::adapter_weak_ptr  adapter_;
    publisher_ptr           publisher_;
};

}  /* namespace svc */
}  /* namespace wire */

#endif /* WIRE_BUS_BUS_IMPL_HPP_ */
