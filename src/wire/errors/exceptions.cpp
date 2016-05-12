/*
 * exceptions.cpp
 *
 *  Created on: May 12, 2016
 *      Author: zmij
 */

#include <wire/errors/exceptions.hpp>
#include <wire/errors/not_found.hpp>

#include <iostream>

namespace wire {
namespace errors {

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

