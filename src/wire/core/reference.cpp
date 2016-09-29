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
#include <wire/core/locator.hpp>

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

reference_data
operator "" _wire_ref(char const* str, ::std::size_t sz)
{
    ::std::string literal{str, sz};
    ::std::istringstream is{literal};
    reference_data ref;
    if (!(is >> ref))
        throw ::std::runtime_error{"Invalid ::wire::core::reference_data literal " + literal};
    return ref;
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
            ::std::make_shared< fixed_reference >(cnctr, ref_data);
    } else if (ref_data.adapter.is_initialized()) {
        return ::std::make_shared< floating_reference >(cnctr, ref_data);
    }
    auto lctr = cnctr->get_locator();
    if (lctr) {
        auto prx = lctr->find_object(ref_data.object_id);
        return ::std::make_shared< fixed_reference >( cnctr,
                    prx->wire_get_reference()->data());
    }
    throw errors::runtime_error{"wire.locator is not configured"};
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

connector_ptr
reference::get_connector() const
{
    auto cnctr = connector_.lock();
    if (!cnctr)
        throw errors::runtime_error{ "Connector already destroyed" };
    return cnctr;
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
    : reference{cn, ref}, current_{ref_.endpoints.end()}
{
    if (ref_.endpoints.empty())
        throw errors::runtime_error{ "Reference endpoint list is empty" };
}

fixed_reference::fixed_reference(fixed_reference const& rhs)
    : reference{rhs},
      connection_{rhs.connection_.lock()},
      current_{ref_.endpoints.end()}
{
}

fixed_reference::fixed_reference(fixed_reference&& rhs)
    : reference{::std::move(rhs)},
      connection_{rhs.connection_.lock()},
      current_{ref_.endpoints.end()}
{
}

void
fixed_reference::get_connection_async( connection_callback on_get,
        functional::exception_callback on_error,
        bool sync) const
{
    connection_ptr conn = connection_.lock();
    if (!conn) {
        lock_guard lock{mutex_};
        conn = connection_.lock();
        if (conn) {
            try {
                on_get(conn);
            } catch (...) {}
            return;
        }
        connector_ptr cntr = get_connector();
        if (current_ == ref_.endpoints.end()) {
            current_ = ref_.endpoints.begin();
        }
        auto _this = shared_this<fixed_reference>();
        cntr->get_outgoing_connection_async(
            *current_++,
            [_this, on_get, on_error](connection_ptr c)
            {
                auto conn = _this->connection_.lock();
                if (!conn || conn != c) {
                    _this->connection_ = c;
                    conn = c;
                }
                try {
                    on_get(conn);
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
        try {
            on_get(conn);
        } catch(...) {}
    }
}

//----------------------------------------------------------------------------
//      Floating reference implementation
//----------------------------------------------------------------------------

floating_reference::floating_reference(connector_ptr cn, reference_data const& ref)
    : reference{ cn, ref }
{
}

floating_reference::floating_reference(floating_reference const& rhs)
    : reference{rhs}
{
}

floating_reference::floating_reference(floating_reference&& rhs)
    : reference{ ::std::move(rhs) }
{
}

void
floating_reference::get_connection_async( connection_callback on_get,
        functional::exception_callback exception,
        bool sync ) const
{
    rotation_type_ptr rot;
    {
        lock_guard lock{mutex_};
        rot = cache_;
    }
    if (rot) {
        next_connection(rot, on_get, exception, sync);
    } else {
        connector_ptr cnctr = get_connector();
        auto _this = shared_this<floating_reference>();
        cnctr->get_locator_async(
            [_this, on_get, exception, sync](locator_prx loc)
            {
                if (!loc) {
                    if (exception) {
                        try {
                            exception(::std::make_exception_ptr(errors::runtime_error{"Locator cannot be reached"}));
                        } catch (...) {}
                    }
                } else {
                    loc->find_adapter_async(_this->ref_.object_id,
                        [_this, on_get, exception, sync](object_prx prx)
                        {
                            {
                                lock_guard lock{_this->mutex_};
                                _this->cache_ = ::std::make_shared<rotation_type>(prx->wire_get_reference()->data().endpoints);
                            }
                            _this->next_connection(_this->cache_, on_get, exception, sync);
                        },
                        [exception](::std::exception_ptr ex)
                        {
                            if (exception) {
                                try {
                                    exception(ex);
                                } catch (...) {}
                            }
                        }, nullptr, no_context, sync);
                }
            },
            [exception](::std::exception_ptr ex)
            {
                if (exception) {
                    try {
                        exception(ex);
                    } catch (...) {}
                }
            },
            no_context, sync);
    }
}

void
floating_reference::next_connection(rotation_type_ptr rot,
        connection_callback on_get,
        functional::exception_callback on_error,
        bool sync) const
{
    if (!rot || rot->empty()) {
        if (on_error)
            try {
                on_error(::std::make_exception_ptr(errors::connection_failed{ "Failed to obtain endpoints" }));
            } catch (...) {}
    } else {
        connector_ptr cnctr = get_connector();
        auto _this = shared_this<floating_reference>();
        cnctr->get_outgoing_connection_async(rot->next(), on_get, on_error, sync);
    }
}


}  // namespace core
}  // namespace wire
