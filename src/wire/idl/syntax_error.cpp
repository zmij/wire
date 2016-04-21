/*
 * syntax_error.cpp
 *
 *  Created on: Apr 21, 2016
 *      Author: zmij
 */

#include <wire/idl/syntax_error.hpp>
#include <iostream>
#include <sstream>

namespace wire {
namespace idl {

::std::ostream&
operator << (::std::ostream& os, error_level const& val)
{
    ::std::ostream::sentry s(os);
    if (s) {
        switch (val) {
            case error_level::warning:
                os << "warning";
                break;
            case error_level::error:
                os << "error";
                break;
        }
    }
    return os;
}


::std::string
format_error_message(source_location const& loc, error_level lvl, ::std::string const& msg)
{
    ::std::ostringstream os;
    os << loc << " " << lvl << ": " << msg;
    return os.str();
}


}  /* namespace idl */
}  /* namespace wire */
