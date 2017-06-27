/*
 * context.cpp
 *
 *  Created on: Jun 27, 2017
 *      Author: zmij
 */

#include <wire/core/context.hpp>

namespace wire {
namespace core {

namespace {

::std::string const NO_DATA{""};

} /* namespace  */

::std::string const&
context_type::operator [](::std::string const& key) const
{
    auto f = data_.find(key);
    if (f != data_.end()) {
        return f->second;
    }
    return NO_DATA;
}

} /* namespace core */
} /* namespace wire */

