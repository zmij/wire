/*
 * bus_service.hpp
 *
 *  Created on: 16 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_BUS_BUS_SERVICE_HPP_
#define WIRE_BUS_BUS_SERVICE_HPP_

#include <wire/core/service.hpp>
#include <wire/core/proxy_fwd.hpp>

#include <wire/bus/bus_fwd.hpp>

#include <string>
#include <memory>

namespace wire {
namespace svc {

class bus_service : public core::service {
public:
    bus_service(::std::string const& name = "wire.bus");
    virtual
    ~bus_service();

    void
    start(core::connector_ptr cnctr) override;
    void
    stop() override;

    bus::bus_registry_prx
    registry() const;
private:
    struct impl;
    using pimpl = ::std::unique_ptr<impl>;
    pimpl pimpl_;
};

}  /* namespace svc */
}  /* namespace wire */


#endif /* WIRE_BUS_BUS_SERVICE_HPP_ */
