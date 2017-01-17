/*
 * bus_options.hpp
 *
 *  Created on: 16 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_BUS_BUS_OPTIONS_HPP_
#define WIRE_BUS_BUS_OPTIONS_HPP_

#include <wire/core/endpoint.hpp>
#include <wire/core/reference.hpp>

namespace wire {
namespace svc {

struct bus_options {
    /**
     * Bus registry object identity
     */
    core::identity      bus_registry_id     = "bus.registry"_wire_id;
    /**
     * Category and replica name for adapter hosting registry and bus objects
     */
    ::std::string       bus_adapter_category = "wire.bus";
    /**
     * Endpoints for adapter hosting registry and bus objects
     */
    core::endpoint_list bus_endpoints;
    /**
     * Category for bus objects
     */
    ::std::string       bus_category         = "bus";
    /**
     * Category for adapter and replica name
     */
    ::std::string       publisher_adapter_category = "wire.bus.publisher";
    /**
     * Endpoints for adapter hosting publisher objects
     */
    core::endpoint_list publisher_endpoints;
    /**
     * List of bus objects registered at startup
     */
    core::identity_seq  predefined_buses;
    /**
     * Allow creation of new bus objects
     */
    bool                allow_create        = true;

    bool                collocate_registry  = false;
    /**
     * Print bus registry proxy after service initialization
     */
    bool                print_proxy         = false;
};

} /* namespace svc */
} /* namespace wire */

#endif /* WIRE_BUS_BUS_OPTIONS_HPP_ */
