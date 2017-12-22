/*
 * client.cpp
 *
 *  Created on: Dec 20, 2017
 *      Author: zmij
 */

#include <wire/bus/client.hpp>

#include <wire/core/connector.hpp>
#include <wire/core/proxy.hpp>

#include <wire/bus/bus.hpp>

#include <tbb/concurrent_hash_map.h>

namespace wire {
namespace bus {

using core::functional::report_exception;

struct client::impl : ::std::enable_shared_from_this<impl> {
    using bus_cache     = ::tbb::concurrent_hash_map<core::identity, bus_prx>;

    bus_registry_prx    bus_reg_;
    bus_cache           cache_;

    impl() {}
    impl(bus_registry_prx bus_reg)
        : bus_reg_{ bus_reg }
    {
    }
    impl(core::connector_ptr conn, core::reference_data const& ref)
        : bus_reg_{ core::unchecked_cast< bus_registry_proxy >( conn->make_proxy(ref) ) }
    {
    }

    void
    get_bus_async(core::identity const& bus_id,
            core::functional::callback< bus_prx >   __resp,
            core::functional::exception_callback    __exception,
            core::invocation_options                _opts)
    {
        bus_cache::const_accessor acc;
        if (cache_.find(acc, bus_id)) {
            __resp(acc->second);
        } else {
            if (bus_reg_) {
                bus_reg_->get_or_create_bus_async(bus_id,
                    ::std::bind(&impl::cache_and_proceed, shared_from_this(),
                            ::std::placeholders::_1, bus_id, __resp, __exception),
                    __exception, nullptr, core::no_context, _opts);
            } else {
                report_exception(__exception,
                        ::std::runtime_error{ "Bus registry is empty" });
            }
        }
    }

    void
    cache_and_proceed(bus_prx bus, core::identity const& bus_id,
            core::functional::callback< bus_prx > __resp,
            core::functional::exception_callback __exception)
    {
        if (bus) {
            {
                bus_cache::accessor acc;
                if (!cache_.find(acc, bus_id)) {
                    cache_.insert({ bus_id, bus });
                }
            }
            __resp(bus);
        } else {
            report_exception(__exception, no_bus{bus_id});
        }
    }

    void
    subscribe_async(core::identity const& bus_id, core::object_prx obj,
            core::functional::void_callback         __resp,
            core::functional::exception_callback    __exception,
            core::invocation_options                _opts = core::invocation_options::unspecified)
    {
        get_bus_async(bus_id,
            ::std::bind(&impl::do_subscribe_async, shared_from_this(),
                    ::std::placeholders::_1, bus_id, obj, __resp, __exception, _opts),
                     __exception, _opts);
    }

    void
    do_subscribe_async(bus_prx bus, core::identity const& bus_id, core::object_prx obj,
            core::functional::void_callback         __resp,
            core::functional::exception_callback    __exception,
            core::invocation_options                _opts)
    {
        if (bus) {
            bus->subscribe_async(obj, __resp, __exception,
                    nullptr, core::no_context, _opts);
        } else {
            report_exception(__exception, no_bus{bus_id});
        }
    }

    void
    unsubscribe_async(core::identity const& bus_id, core::object_prx obj,
            core::functional::void_callback         __resp,
            core::functional::exception_callback    __exception,
            core::invocation_options                _opts = core::invocation_options::unspecified)
    {
        get_bus_async(bus_id,
            ::std::bind(&impl::do_unsubscribe_async, shared_from_this(),
                    ::std::placeholders::_1, bus_id, obj, __resp, __exception, _opts),
                     __exception, _opts);
    }

    void
    do_unsubscribe_async(bus_prx bus, core::identity const& bus_id, core::object_prx obj,
            core::functional::void_callback         __resp,
            core::functional::exception_callback    __exception,
            core::invocation_options                _opts)
    {
        if (bus) {
            bus->unsubscribe_async(obj, __resp, __exception, nullptr, core::no_context, _opts);
        } else {
            report_exception(__exception, no_bus{bus_id});
        }
    }

    void
    get_publisher_async(core::identity const& bus_id,
            core::functional::callback<core::object_prx> __resp,
            core::functional::exception_callback    __exception,
            core::invocation_options                _opts = core::invocation_options::unspecified)
    {
        get_bus_async(bus_id,
            ::std::bind(&impl::do_get_publisher_async, shared_from_this(),
                    ::std::placeholders::_1, bus_id, __resp, __exception, _opts),
                     __exception, _opts);
    }

    void
    do_get_publisher_async(bus_prx bus, core::identity const& bus_id,
            core::functional::callback<core::object_prx> __resp,
            core::functional::exception_callback    __exception,
            core::invocation_options                _opts)
    {
        if (bus) {
            bus->get_publisher_async(__resp, __exception, nullptr, core::no_context, _opts);
        } else {
            report_exception(__exception, no_bus{bus_id});
        }
    }
};

client::client()
    : pimpl_{ ::std::make_shared< impl >() }
{
}

client::client(core::connector_ptr conn, core::reference_data const& ref)
    : pimpl_{ ::std::make_shared< impl >(conn, ref) }
{
}

client::client(bus_registry_prx bus_reg)
    : pimpl_{ ::std::make_shared< impl >(bus_reg) }
{
}

client&
client::operator =(client const& rhs)
{
    client{rhs}.swap(*this);
    return *this;
}

client&
client::operator = (client&& rhs)
{
    rhs.swap(*this);
    return *this;
}

client::operator bool() const
{
    return pimpl_->bus_reg_.get();
}

bool
client::operator !() const
{
    return !pimpl_->bus_reg_.get();
}

bus_prx
client::get_bus(core::identity const& bus_id,
        core::invocation_options _opts) const
{
    auto future = get_bus_async(bus_id, _opts | core::invocation_flags::sync);
    return future.get();
}

void
client::get_bus_async(core::identity const& bus_id,
        core::functional::callback<bus_prx> result,
        core::functional::exception_callback exception,
        core::invocation_options _opts) const
{
    pimpl_->get_bus_async(bus_id, result, exception, _opts);
}

void
client::subscribe(core::identity const& bus_id, core::object_prx prx,
        core::invocation_options _opts) const
{
    auto future = subscribe_async(bus_id, prx, _opts | core::invocation_flags::sync);
    future.get();
}

void
client::subscribe_async(core::identity const& bus_id, core::object_prx prx,
        core::functional::void_callback __resp,
        core::functional::exception_callback __exception,
        core::invocation_options _opts) const
{
    pimpl_->subscribe_async(bus_id, prx, __resp, __exception, _opts);
}

void
client::unsubscribe(core::identity const& bus_id, core::object_prx prx,
        core::invocation_options _opts) const
{
    auto future = unsubscribe_async(bus_id, prx, _opts | core::invocation_flags::sync);
    future.get();
}


void
client::unsubscribe_async(core::identity const& bus_id, core::object_prx prx,
        core::functional::void_callback __resp,
        core::functional::exception_callback __exception,
        core::invocation_options _opts) const
{
    pimpl_->unsubscribe_async(bus_id, prx, __resp, __exception);
}

core::object_prx
client::get_publisher(core::identity const& bus_id,
        core::invocation_options _opts) const
{
    auto future = get_publisher_async(bus_id, _opts | core::invocation_flags::sync);
    return future.get();
}


void
client::get_publisher_async(core::identity const& bus_id,
        core::functional::callback<core::object_prx> __resp,
        core::functional::exception_callback __exception,
        core::invocation_options _opts) const
{
    pimpl_->get_publisher_async(bus_id, __resp, __exception, _opts);
}

} /* namespace bus */
} /* namespace wire */


