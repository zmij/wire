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

#include <wire/util/io_service_wait.hpp>
#include <wire/util/scheduled_task.hpp>

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

::std::size_t
id_facet_hash(reference_data const& ref)
{
    auto h = hash(ref.object_id);
    return (h << 1) ^ hash(ref.facet);
}

::std::size_t
hash(reference_data const& ref)
{
    auto h = id_facet_hash(ref);
    if (ref.adapter.is_initialized())
        h = (h << 1) ^ hash(ref.adapter.get());
    h = (h << 1) ^ hash(ref.endpoints);
    return h;
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
    }
    return ::std::make_shared< floating_reference >(cnctr, ref_data);
}

bool
reference::is_local() const
{
    if (local_object_cache_.lock())
        return true;
    return get_connector()->is_local(*this);
}

reference::local_servant
reference::get_local_object() const
{
    auto obj = local_object_cache_.lock();
    if (!obj) {
        auto loc_srv = get_connector()->find_local_servant(*this);
        obj = loc_srv.first;
        local_object_cache_ = obj;
        adapter_cache_      = loc_srv.second;
    }
    return {obj, adapter_cache_.lock()};
}

connector_ptr
reference::get_connector() const
{
    auto cnctr = connector_.lock();
    if (!cnctr)
        throw errors::connector_destroyed{ "Connector already destroyed" };
    return cnctr;
}

asio_config::io_service_ptr
reference::io_service() const
{
    return get_connector()->io_service();
}

locator_prx
reference::get_locator() const
{
    return ref_.locator;
}

void
reference::set_locator(locator_prx lctr)
{
    ref_.locator = lctr;
}

void
reference::set_locator(reference_data const& loc_ref)
{
    ref_.locator = unchecked_cast<locator_proxy>(get_connector()->make_proxy(loc_ref));
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
fixed_reference::get_connection_async(
        connection_callback             __result,
        functional::exception_callback  __exception,
        invocation_options const&       in_opts) const
{
    connection_ptr conn = connection_.lock();
    if (!conn) {
        connector_ptr cntr = get_connector();
        endpoint ep;
        {
            lock_guard lock{mutex_};
            conn = connection_.lock();
            if (current_ == ref_.endpoints.end()) {
                current_ = ref_.endpoints.begin();
            }
            ep = *current_++;
        }
        if (conn) {
            // Connection is already there
            functional::report_async_result(__result, conn);
            return;
        }

        auto opts = in_opts;
        if (opts.is_sync())
            opts ^= invocation_flags::sync;

        auto _this = shared_this<fixed_reference>();
        auto res = ::std::make_shared< ::std::atomic<bool> >(false);
        if (!opts.dont_retry() && in_opts.is_sync()) {
            auto err = [__exception, res](::std::exception_ptr ex)
                {
                    *res = true;
                    functional::report_exception(__exception, ex);
                };
            __exception = err;
        }

        auto get_connection =
            [_this, __result, __exception, res](connection_ptr c)
            {
                connection_ptr conn;
                {
                    lock_guard lock{_this->mutex_};
                    conn = _this->connection_.lock();
                    if (!conn || conn != c) {
                        _this->connection_ = c;
                        conn = c;
                    }
                }
                try {
                    *res = true;
                    __result(conn);
                } catch(...) {
                    try {
                        __exception(::std::current_exception());
                    } catch(...) {}
                }
            };

        functional::exception_callback connect_error;

        if (opts.dont_retry()) {
            connect_error =
                [__exception, res](::std::exception_ptr ex)
                {
                    *res = true;
                    functional::report_exception(__exception, ex);
                };
        } else {
            functional::void_callback retry_func;
            if (opts.retries == invocation_options::infinite_retries) {
                retry_func =
                    [_this, get_connection, __exception, opts]()
                    {
                        _this->get_connection_async(get_connection, __exception, opts);
                    };
            } else {
                retry_func =
                    [_this, get_connection, __exception, opts]()
                    {
                        _this->get_connection_async(get_connection, __exception, opts.dec_retries());
                    };
            }
            connect_error =
                [_this, retry_func, __exception, opts, res](::std::exception_ptr ex)
                {
                    try {
                        ::std::rethrow_exception(ex);
                    } catch (errors::connection_refused const& ex) {
                        // Retry connection
                        util::schedule_in(
                            _this->get_connector()->io_service(),
                            [retry_func, __exception, res]( asio_config::error_code const& ec )
                            {
                                if (!ec) {
                                    retry_func();
                                } else {
                                    functional::report_exception(__exception, ec);
                                }
                            }, ::std::chrono::milliseconds{ opts.retry_timeout });
                    } catch(...) {
                        functional::report_exception(__exception, ::std::current_exception());
                    }
                };
        }

        cntr->get_outgoing_connection_async(ep, get_connection, connect_error, opts);
        if (in_opts.is_sync()) {
            util::run_until(_this->get_connector()->io_service(), [res](){ return (bool)*res; });
        }
    } else {
        try {
            __result(conn);
        } catch(...) {}
    }
}

//----------------------------------------------------------------------------
//      Floation_geteference implementation
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
floating_reference::get_connection_async( connection_callback __result,
        functional::exception_callback __exception,
        invocation_options const& opts) const
{
    connector_ptr cnctr = get_connector();
    cnctr->resolve_connection_async(ref_, __result, __exception, opts);
}


}  // namespace core
}  // namespace wire
