/*
 * sparring_options.hpp
 *
 *  Created on: 27 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef TRANSPORT_SPARRING_OPTIONS_HPP_
#define TRANSPORT_SPARRING_OPTIONS_HPP_

#include <string>
#include <wire/core/endpoint.hpp>

namespace wire {
namespace test {

class sparring_options {
public:
	wire::core::transport_type	transport = wire::core::transport_type::empty;

	std::size_t					connections			= 0;
	std::size_t					requests			= 1;

	std::string					cert_file;
	std::string					key_file;
	std::string					verify_file;

	bool						validate_message	= false;
	bool						require_peer_cert	= false;

	static sparring_options&
	instance();
private:
	sparring_options() {}
	sparring_options(sparring_options const&) = delete;
	sparring_options&
	operator =(sparring_options const&) = delete;
};

} /* namespace test */
} /* namespace wire */

#endif /* TRANSPORT_SPARRING_OPTIONS_HPP_ */
