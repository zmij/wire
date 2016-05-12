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

namespace wire {
namespace core {
namespace detail {

struct ssl_options {
	::std::string	verify_file;
	::std::string	cert_file;
	::std::string	key_file;
	bool			require_peer_cert;
};


struct connector_options {
	/**
	 * Connector name
	 */
	::std::string	name		= "wire.connector";
	/**
	 * Configuration file
	 */
	::std::string	config_file;
	/**
	 * Server-side ssl options
	 */
	ssl_options		server_ssl;
	/**
	 * Client-side ssl options
	 */
	ssl_options		client_ssl;

	connector_options() {}
	connector_options(::std::string const& name) : name(name) {}
};

struct adapter_options {
	endpoint_list	endpoints;
	::std::size_t	timeout;
	ssl_options		adapter_ssl;
};

}  // namespace detail
}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_DETAIL_CONFIGURATION_OPTIONS_HPP_ */
