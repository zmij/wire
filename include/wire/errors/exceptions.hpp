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
#include <iosfwd>

namespace wire {
namespace errors {

/**
 * Base class for all wire runtime exceptions
 */
class runtime_error : public ::std::runtime_error {
public:
    runtime_error(std::string const& msg ) :
        ::std::runtime_error{""}, message_{msg} {}
    runtime_error(char const* msg)
        : ::std::runtime_error{""}, message_{msg} {}

    template < typename ... T >
    runtime_error(T const& ... args)
        : ::std::runtime_error{""}, message_{ util::concatenate(args ...) } {}
    char const*
    what() const noexcept override;
protected:
    virtual void
    stream_message(::std::ostream& os) const {}
private:
    ::std::string mutable message_;
};

class connector_destroyed : public runtime_error {
public:
    connector_destroyed(std::string const& msg) : runtime_error{msg} {}
    connector_destroyed(char const* msg) : runtime_error{msg} {}
};

class adapter_destroyed : public runtime_error {
public:
    adapter_destroyed(std::string const& msg) : runtime_error{msg} {}
    adapter_destroyed(char const* msg) : runtime_error{msg} {}
};

class no_locator : public runtime_error {
public:
    no_locator() : runtime_error{"Locator is not configured"} {}
};


class connection_failed : public runtime_error {
public:
    connection_failed(std::string const& message) : runtime_error(message) {}
    template < typename ... T >
    connection_failed(T const& ... args) : runtime_error(args ...) {}
};

class request_timed_out : public runtime_error {
public:
    request_timed_out(std::string const& msg) : runtime_error{msg} {}
    template < typename ... T >
    request_timed_out(T const& ... args) : runtime_error(args ...) {}
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

}  // namespace errors
}  // namespace wire



#endif /* WIRE_ERRORS_EXCEPTIONS_HPP_ */
