/*
 * object_locator.hpp
 *
 *  Created on: 13 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_CORE_OBJECT_LOCATOR_HPP_
#define WIRE_CORE_OBJECT_LOCATOR_HPP_

#include <wire/core/object_fwd.hpp>
#include <wire/core/identity_fwd.hpp>
#include <wire/core/adapter_fwd.hpp>
#include <string>

namespace wire {
namespace core {

class object_locator {
public:
    virtual ~object_locator() {};
    virtual object_ptr
    find_object( adapter_ptr,
            identity const& id, ::std::string const& facet ) = 0;
};

}  /* namespace core */
}  /* namespace wire */

#endif /* WIRE_CORE_OBJECT_LOCATOR_HPP_ */
