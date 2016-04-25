/*
 * type_name.cpp
 *
 *  Created on: 25 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/idl/type_name.hpp>
#include <iostream>

namespace wire {
namespace idl {

std::ostream&
operator << (std::ostream& os, type_name const& val)
{
    std::ostream::sentry s (os);
    if (s) {
        os << val.name;
        if (!val.params.empty()) {
            os << "< ";
            for (auto p = val.params.begin(); p != val.params.end(); ++p) {
                if (p != val.params.begin())
                    os << ", ";
                switch (p->which()) {
                    case 0:
                        os << *::boost::get< type_name::type_name_ptr >(*p);
                        break;
                    default:
                        os << ::boost::get< ::std::string >(*p);
                        break;
                }
            }
            os << " >";
            if (val.is_reference)
                os << "*";
        }
    }
    return os;
}


}  /* namespace idl */
}  /* namespace wire */
