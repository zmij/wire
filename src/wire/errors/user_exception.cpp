/*
 * user_exception.cpp
 *
 *  Created on: May 9, 2016
 *      Author: zmij
 */

#include <wire/errors/user_exception.hpp>
#include <sstream>

namespace wire {
namespace errors {

namespace {

::std::string const WIRE_ERRORS_USER_EXCEPTION_TYPE_ID = "::wire::errors::user_exception";

}  /* namespace  */

::std::string const&
user_exception::wire_static_type_id()
{
    return WIRE_ERRORS_USER_EXCEPTION_TYPE_ID;
}

void
user_exception::post_unmarshal()
{
    ::std::ostringstream os;
    stream_message(os);
    message_ = os.str();
}

}  /* namespace errors */
}  /* namespace wire */
