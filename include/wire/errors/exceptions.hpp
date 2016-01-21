/*
 * base_errors.hpp
 *
 *  Created on: 21 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_ERRORS_EXCEPTIONS_HPP_
#define WIRE_ERRORS_EXCEPTIONS_HPP_

#include <stdexcept>

namespace wire {
namespace errors {

/**
 * Base class for all wire runtime exceptions
 */
class runtime_error : public ::std::runtime_error {
public:
	runtime_error(std::string const& message ) : ::std::runtime_error(message) {}
};

class invalid_magic_number : public runtime_error {
public:
	invalid_magic_number(std::string const& message) : runtime_error(message) {}
};

class unmarshal_error : public runtime_error {
public:
	unmarshal_error(std::string const& message) : runtime_error(message) {}
};

}  // namespace errors
}  // namespace wire



#endif /* WIRE_ERRORS_EXCEPTIONS_HPP_ */
