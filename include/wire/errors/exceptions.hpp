/*
 * base_errors.hpp
 *
 *  Created on: 21 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_ERRORS_EXCEPTIONS_HPP_
#define WIRE_ERRORS_EXCEPTIONS_HPP_

#include <stdexcept>
#include <wire/util/concatenate.hpp>

namespace wire {
namespace errors {

/**
 * Base class for all wire runtime exceptions
 */
class runtime_error : public ::std::runtime_error {
public:
	runtime_error(std::string const& message ) : ::std::runtime_error(message) {}

	template < typename ... T >
	runtime_error(T const& ... args)
		: ::std::runtime_error( util::concatenate(args ...) ) {}
};

class connection_failed : public runtime_error {
public:
	connection_failed(std::string const& message) : runtime_error(message) {}
};

class invalid_magic_number : public runtime_error {
public:
	invalid_magic_number(std::string const& message) : runtime_error(message) {}
};

class marshal_error : public runtime_error {
public:
	marshal_error(std::string const& message) : runtime_error(message) {}
};

class unmarshal_error : public runtime_error {
public:
	unmarshal_error(std::string const& message) : runtime_error(message) {}
};

class logic_error : public ::std::logic_error {
public:
	logic_error(std::string const& message ) : ::std::logic_error(message) {}
	template < typename ... T >
	logic_error(T const& ... args)
		: ::std::logic_error( util::concatenate(args ...) ) {}
};

}  // namespace errors
}  // namespace wire



#endif /* WIRE_ERRORS_EXCEPTIONS_HPP_ */
