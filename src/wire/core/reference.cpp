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

bool
operator == (reference_data const& lhs, reference_data const& rhs)
{
    return lhs.object_id == rhs.object_id &&
            lhs.facet == rhs.facet &&
            lhs.adapter == rhs.adapter &&
            lhs.endpoints == rhs.endpoints;
}
bool
operator != (reference_data const& lhs, reference_data const& rhs)
{
    return !(lhs == rhs);
}
bool
operator < (reference_data const& lhs, reference_data const& rhs)
{
    return lhs.object_id < rhs.object_id &&
            lhs.facet < rhs.facet &&
            lhs.adapter < rhs.adapter &&
            lhs.endpoints < rhs.endpoints;
}


//----------------------------------------------------------------------------
//      Base reference implementation
//----------------------------------------------------------------------------
reference_ptr
reference::create_reference(connector_ptr cnctr, reference_data const& ref_data)
{
    if (!ref_data.endpoints.empty()) {
        // Find a connection or create a new one
        return
            ::std::make_shared< fixed_reference >( cnctr, ref_data);
    } else if (ref_data.adapter.is_initialized()) {
        throw errors::runtime_error("Adapter location is not implemented yet");
    }
    throw errors::runtime_error{"Well-known objects are not implemented yet"};
}

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
