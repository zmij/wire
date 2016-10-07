/*
 * connector_options.hpp
 *
 *  Created on: Mar 1, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_DETAIL_CONFIGURATION_OPTIONS_HPP_
#define WIRE_CORE_DETAIL_CONFIGURATION_OPTIONS_HPP_

#include <string>
#include <wire/core/endpoint.hpp>
#include <wire/core/reference.hpp>
#include <wire/core/detail/ssl_options.hpp>

namespace wire {
namespace core {
namespace detail {

struct connector_options {
    /**
     * Connector name
     */
    ::std::string   name                = "wire.connector";
    /**
     * Configuration file
     */
    ::std::string   config_file;
    /**
     * Locator reference
     */
    reference_data  locator_ref         = reference_data{};
    /**
     * Endpoints for admin interface
     */
    endpoint_list   admin_endpoints;
    identity        admin_adapter       = "admin";
    identity        admin_connector     = "connector";

    /**
     * Server-side ssl options
     */
    ssl_options     server_ssl;
    /**
     * Client-side ssl options
     */
    ssl_options     client_ssl;

    connector_options() {}
    connector_options(::std::string const& name) : name(name) {}
};

struct adapter_options {
    /**
     * Adapter listen endpoints
     */
    endpoint_list   endpoints;
    ::std::size_t   timeout;
    bool            registered;
    bool            replicated;
    ssl_options     adapter_ssl;
};

}  // namespace detail
}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_DETAIL_CONFIGURATION_OPTIONS_HPP_ */
