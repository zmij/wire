/*
 * exceptions.cpp
 *
 *  Created on: May 12, 2016
 *      Author: zmij
 */

#include <wire/errors/exceptions.hpp>
#include <wire/errors/not_found.hpp>

#include <iostream>
#include <sstream>

namespace wire {
namespace errors {

char const*
runtime_error::what() const noexcept
{
    if (message_.empty()) {
        // call stream message
        ::std::ostringstream os;
        stream_message(os);
        message_ = os.str();
    }
    return message_.c_str();
}

void
not_found::stream_message(::std::ostream& os) const
{
    os << format_message(subj_, object_id_, facet_, operation_);
}

::std::string
not_found::format_message(subject s, core::identity const& obj_id,
        ::std::string const& f, ::std::string const& op)
{
    ::std::ostringstream os;

    switch (s) {
        case object:
            os << "Object ";
            break;
        case facet:
            os << "Facet ";
            break;
        case operation:
            os << "Operation ";
            break;
        default:
            break;
    }

    os << "not found. Id: " << obj_id << " Facet: " << f << " Operation: " << op;

    return os.str();
}

}  /* namespace errors */
}  /* namespace wire */

