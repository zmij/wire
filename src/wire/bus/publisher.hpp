/*
 * publisher.hpp
 *
 *  Created on: 7 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_BUS_PUBLISHER_HPP_
#define WIRE_BUS_PUBLISHER_HPP_

#include <wire/bus/bus.hpp>
#include <wire/core/object.hpp>
#include <memory>

namespace wire {
namespace svc {

class publisher : public core::object {
public:
    publisher();
    virtual
    ~publisher();

public:
    //@{
    /** @name Wire object functions
     *  Functions are overloaded to throw no_operation when required to
     *  return value.
     *  Ping will run OK if publisher exists.
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

    //{
    void
    add_bus(core::identity const&);
    void
    remove_bus(core::identity const&);
    bool
    bus_exists(core::identity const&) const;

    void
    add_subscriber(core::identity const& bus, core::object_prx,
            core::current const&);
    void
    remove_subscriber(core::identity const& bus, core::object_prx,
            core::current const&);
    //@}
protected:

    bool
    __wire_dispatch(core::detail::dispatch_request const&, core::current const&,
            dispatch_seen_list&, bool throw_not_found) override;
private:
    struct impl;
    typedef ::std::unique_ptr<impl> pimpl;
    pimpl pimpl_;
};

using publisher_ptr = ::std::shared_ptr< publisher >;

}  /* namespace svc */
}  /* namespace wire */


#endif /* WIRE_BUS_PUBLISHER_HPP_ */
