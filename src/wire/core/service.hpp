/*
 * wire_service.hpp
 *
 *  Created on: Sep 27, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_SERVICE_HPP_
#define WIRE_CORE_SERVICE_HPP_

#include <wire/core/connector_fwd.hpp>

namespace wire {
namespace core {

class service {
public:
    virtual
    ~service() {}

    virtual void
    start(connector_ptr cnctr) = 0;
    virtual void
    stop() {}
};

}  /* namespace core */
}  /* namespace wire */

#endif /* WIRE_CORE_SERVICE_HPP_ */
