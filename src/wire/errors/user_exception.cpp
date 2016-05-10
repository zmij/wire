/*
 * user_exception.cpp
 *
 *  Created on: May 9, 2016
 *      Author: zmij
 */

#include <wire/errors/user_exception.hpp>
#include <mutex>

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

}  /* namespace errors */
}  /* namespace wire */
