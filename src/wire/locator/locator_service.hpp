/*
 * locator_service.hpp
 *
 *  Created on: Sep 27, 2016
 *      Author: zmij
 */

#ifndef WIRE_LOCATOR_LOCATOR_SERVICE_HPP_
#define WIRE_LOCATOR_LOCATOR_SERVICE_HPP_

#include <wire/core/service.hpp>

#include <string>
#include <memory>

namespace wire {
namespace svc {

class locator_service : public core::service {
public:
    locator_service(::std::string const& name = "wire.locator");
    virtual
    ~locator_service();

    void
    start(core::connector_ptr cnctr) override;
    void
    stop() override;

    ::std::string const&
    name() const;
    void
    name(::std::string const&);
private:
    struct impl;
    using pimpl = ::std::unique_ptr<impl>;
    pimpl pimpl_;
};

} /* namespace svc */
} /* namespace wire */

#endif /* WIRE_LOCATOR_LOCATOR_SERVICE_HPP_ */
