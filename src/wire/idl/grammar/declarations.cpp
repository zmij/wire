/*
 * declarations.cpp
 *
 *  Created on: May 3, 2016
 *      Author: zmij
 */

#include <wire/idl/grammar/declarations.hpp>

namespace wire {
namespace idl {
namespace grammar {

annotation_list::const_iterator
find(annotation_list const& list, ::std::string const& name)
{
    for (auto p = list.begin(); p != list.end(); ++p) {
        if (p->name == name)
            return p;
    }
    return list.end();
}

}  /* namespace grammar */
}  /* namespace idl */
}  /* namespace wire */
