/*
 * user_exception.hpp
 *
 *  Created on: May 3, 2016
 *      Author: zmij
 */

#ifndef WIRE_ERRORS_USER_EXCEPTION_HPP_
#define WIRE_ERRORS_USER_EXCEPTION_HPP_

#include <wire/errors/exceptions.hpp>
#include <wire/encoding/buffers.hpp>

namespace wire {
namespace errors {

class user_exception : public runtime_error {
public:
    using input_iterator = encoding::incoming::const_iterator;
    using output_iterator = ::std::back_insert_iterator<encoding::outgoing>;
public:
    user_exception() : runtime_error{""} {}
    explicit
    user_exception(std::string const& msg) : runtime_error{msg} {}

    template < typename ... T >
    user_exception(T const& ... args)
        : runtime_error(util::delim_concatenate(" ", args ...)) {}

    virtual ~user_exception() {}


    virtual void
    __wire_write(output_iterator out) = 0;

    virtual void
    __wire_read(input_iterator& begin, input_iterator end, bool read_head = true) = 0;
};


}  /* namespace errors */
}  /* namespace wire */

#endif /* WIRE_ERRORS_USER_EXCEPTION_HPP_ */
