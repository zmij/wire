/*
 * locator_options.hpp
 *
 *  Created on: Sep 27, 2016
 *      Author: zmij
 */

#ifndef WIRE_LOCATOR_LOCATOR_OPTIONS_HPP_
#define WIRE_LOCATOR_LOCATOR_OPTIONS_HPP_

#include <wire/core/endpoint.hpp>
#include <wire/core/reference.hpp>

namespace wire {
namespace svc {

struct locator_options {
    //@{
    /** @name Locator object options */
    /**
     * Locator identity
     */
    core::identity  locator_id          = "locator.locator"_wire_id;
    /**
     * Adapter to use for locator object
     */
    core::identity  locator_adapter     = "locator"_wire_id;
    /**
     * Endpoints for locator adapter object
     */
    core::endpoint_list locator_endpoints;
    //@}
    //@{
    /** @name Locator registry object options */
    /**
     * Locator registry identity
     */
    core::identity  registry_id         = "locator.registry"_wire_id;
    /**
     * Adapter to use for registry object. If the same as locator adapter,
     * no additional adapter will be created.
     */
    core::identity  registry_adapter    = "locator"_wire_id;
    /**
     * Endpoints for registry object adapter. Will be ignored if registry adapter
     * is the same with locator adapter.
     */
    core::endpoint_list registry_endpoints;
    //@}
};

}  /* namespace svc */
}  /* namespace wire */


#endif /* WIRE_LOCATOR_LOCATOR_OPTIONS_HPP_ */
