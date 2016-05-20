/*
 * reference.cpp
 *
 *  Created on: Apr 12, 2016
 *      Author: zmij
 */

#include <wire/core/reference.hpp>

#include <iostream>
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

bool
reference::is_local() const
{
    if (local_object_cache_.lock())
        return true;
    return get_connector()->is_local(*this);
}

object_ptr
reference::get_local_object() const
{
    auto obj = local_object_cache_.lock();
    if (!obj) {
        obj = get_connector()->find_local_servant(*this);
        local_object_cache_ = obj;
    }
    return obj;
}

connection_ptr
reference::get_connection() const
{
    auto future = get_connection_async(true);
    return future.get();
}

asio_config::io_service_ptr
reference::io_service() const
{
    return get_connector()->io_service();
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

void
fixed_reference::get_connection_async( connection_callback on_get,
        functional::exception_callback on_error,
        bool sync) const
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
        ::std::cerr << "Get fixed reference to " << *current_ << "\n";
        cntr->get_outgoing_connection_async(
            *current_++,
            [this, on_get, on_error](connection_ptr c)
            {
                connection_ = c;
                try {
                    on_get(c);
                } catch(...) {
                    try {
                        on_error(::std::current_exception());
                    } catch(...) {}
                }
            },
            [on_error](::std::exception_ptr ex)
            {
                try {
                    on_error(ex);
                } catch(...) {}
            },
            sync);
    } else {
        ::std::cerr << "Fixed reference is still there\n";
        try {
            on_get(conn);
        } catch(...) {}
    }
}

}  // namespace core
}  // namespace wire
