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
    runtime_error(std::string const& msg ) : ::std::runtime_error(msg) {}
    runtime_error(char const* msg) : ::std::runtime_error{msg} {}

    template < typename ... T >
    runtime_error(T const& ... args)
        : ::std::runtime_error( util::concatenate(args ...) ) {}
};

class connection_failed : public runtime_error {
public:
    connection_failed(std::string const& message) : runtime_error(message) {}
    template < typename ... T >
    connection_failed(T const& ... args) : runtime_error(args ...) {}
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
    template < typename ... T >
    unmarshal_error(T const& ... args) : runtime_error(args ...) {}
};

class logic_error : public ::std::logic_error {
public:
    logic_error(std::string const& message ) : ::std::logic_error(message) {}
    template < typename ... T >
    logic_error(T const& ... args)
        : ::std::logic_error( util::concatenate(args ...) ) {}
};

class not_found : public runtime_error {
public:
    enum subject { object, facet, operation };
public:
    not_found(std::string const& msg, subject s)
        : runtime_error{msg}, subj_(s) {}
    not_found(char const* msg, subject s)
        : runtime_error{msg}, subj_(s) {}
    template < typename ... T >
    not_found(subject s, T const& ... args) :
        runtime_error(util::delim_concatenate(" ", args ...)), subj_(s) {}

    subject
    subj() const
    { return subj_; }
private:
    subject subj_;
};


class no_object : public not_found {
public:
    no_object(std::string const& msg) : not_found{msg, object} {}
    no_object(char const* msg) : not_found{msg, object} {}
    template < typename ... T >
    no_object(T const& ... args) : not_found(object, args ...) {}
};

class no_operation : public not_found {
public:
    no_operation(std::string const& msg) : not_found{msg, operation} {}
    no_operation(char const* msg) : not_found{msg, operation} {}
    template < typename ... T >
    no_operation(T const& ... args) : not_found(operation, args ...) {}
};

class no_facet : public not_found {
public:
    no_facet(std::string const& msg) : not_found{msg, facet} {}
    no_facet(char const* msg) : not_found{msg, facet} {}
    template < typename ... T >
    no_facet(T const& ... args) : not_found(facet, args ...) {}
};


}  // namespace errors
}  // namespace wire



#endif /* WIRE_ERRORS_EXCEPTIONS_HPP_ */
