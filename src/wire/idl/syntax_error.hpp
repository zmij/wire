/*
 * syntax_error.hpp
 *
 *  Created on: Apr 21, 2016
 *      Author: zmij
 */

#ifndef WIRE_IDL_SYNTAX_ERROR_HPP_
#define WIRE_IDL_SYNTAX_ERROR_HPP_

#include <stdexcept>
#include <wire/idl/source_location.hpp>

namespace wire {
namespace idl {

enum class error_level {
    warning,
    error
};

::std::string
format_error_message(source_location const&, error_level, ::std::string const&);

class syntax_error : public ::std::runtime_error {
public:
    syntax_error(source_location const& loc, ::std::string const& message,
            error_level lvl = error_level::error)
        : runtime_error(format_error_message(loc, lvl, message))
    {}
};

class grammar_error : public ::std::runtime_error {
public:
    grammar_error(::std::string const& msg) : runtime_error(msg) {}
};

}  /* namespace idl */
}  /* namespace wire */

#endif /* WIRE_IDL_SYNTAX_ERROR_HPP_ */
