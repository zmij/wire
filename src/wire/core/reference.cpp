/*
 * reference.cpp
 *
 *  Created on: Apr 12, 2016
 *      Author: zmij
 */

#include <wire/core/reference.hpp>

#include <sstream>

#include <wire/core/connector.hpp>

namespace wire {
namespace core {

//----------------------------------------------------------------------------
//      Base reference implementation
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//      Fixed reference implementation
//----------------------------------------------------------------------------
fixed_reference::fixed_reference(connector_ptr cn, reference_data const& ref)
    : reference{cn, ref}, current_{ref_.endpoints.begin()}
{
    if (ref_.endpoints.empty())
        throw errors::runtime_error{ "Reference endpoint list is empty" };
}

connection_ptr
fixed_reference::get_connection() const
{
    connection_ptr conn = connection_.lock();
    if (!conn) {
        connector_ptr cntr = get_connector();
        if (!cntr) {
            throw ::std::runtime_error{"Connector is already destroyed"};
        }
        if (current_ == ref_.endpoints.end()) {
            current_ = ref_.endpoints.begin();
        }
        conn = cntr->get_outgoing_connection(*current_++);
        connection_ = conn;
    }
    return conn;
}

}  // namespace core
}  // namespace wire
