/*
 * publisher.cpp
 *
 *  Created on: 7 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/bus/publisher.hpp>
#include <wire/bus/bus.hpp>
#include <wire/errors/not_found.hpp>
#include <wire/util/make_unique.hpp>

#include <wire/core/detail/dispatch_request.hpp>

#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_unordered_map.h>

#include <sstream>

namespace wire {
namespace svc {

struct publisher::impl {
    struct subscribers {
        using subscriber_map = ::tbb::concurrent_unordered_map<
                                    core::reference_data,
                                    core::object_prx>;

        void
        dispatch(core::detail::dispatch_request const& disp,
                core::current const& curr) const
        {
            // Copy encaps to out buffer
            // Invoke op on all object proxies
        }
        void
        add_subscriber(core::object_prx obj)
        {
            ;
        }

        void
        remove_subscriber(core::object_prx obj)
        {
            ;
        }
    };
    using bus_subscribers = ::tbb::concurrent_hash_map<
                                    core::identity, subscribers >;

    void
    ping(core::current const& curr)
    {
        // Search for publisher id
        bus_subscribers::const_accessor f;
        if (!subscribers_.find(f, curr.operation.identity)) {
            // If none - throw no_object
            throw errors::no_object{
                curr.operation.identity,
                curr.operation.facet,
                "wire_ping"
            };
        }
    }

    void
    dispatch(core::detail::dispatch_request const& disp,
        core::current const& curr) const
    {
        bus_subscribers::const_accessor f;
        if (subscribers_.find(f, curr.operation.identity)) {
            f->second.dispatch(disp, curr);
        } else {
            ::std::ostringstream os;
            os << curr.operation.operation;
            disp.exception( ::std::make_exception_ptr(
                errors::no_object{
                    curr.operation.identity,
                    curr.operation.facet,
                    os.str()
                }));
        }
    }

    void
    add_bus(core::identity const& id)
    {
        bus_subscribers::accessor f;
        if (subscribers_.insert(f, id)) {

        } else {
            throw bus::bus_exists{ id };
        }
    }

    void
    remove_bus(core::identity const& id)
    {
        bus_subscribers::accessor f;
        if (!(subscribers_.find(f, id) && subscribers_.erase(f))) {
            throw bus::no_bus{ id };
        }
    }

    bool
    bus_exists(core::identity const& id) const
    {
        bus_subscribers::accessor f;
        return subscribers_.find(f, id);
    }

    void
    add_subscriber(core::identity const& bus_id, core::object_prx obj,
            core::current const& curr)
    {
        bus_subscribers::accessor f;
        if (subscribers_.find(f, bus_id)) {
            f->second.add_subscriber(obj);
        } else {
            throw errors::no_object{
                curr.operation.identity,
                curr.operation.facet,
                "subscribe"
            };
        }
    }

    void
    remove_subscriber(core::identity const& bus_id, core::object_prx obj,
            core::current const& curr)
    {
        bus_subscribers::accessor f;
        if (subscribers_.find(f, bus_id)) {
            f->second.remove_subscriber(obj);
        } else {
            throw errors::no_object{
                curr.operation.identity,
                curr.operation.facet,
                "unsubscribe"
            };
        }
    }

    bus_subscribers subscribers_;
};

publisher::publisher()
    : pimpl_{util::make_unique<impl>()}
{
}

publisher::~publisher() = default;

bool
publisher::wire_is_a(std::string const&, core::current const& curr) const
{
    throw errors::no_operation{
        curr.operation.identity,
        curr.operation.facet,
        "wire_is_a"
    };
}

void
publisher::wire_ping(core::current const& curr) const
{
    pimpl_->ping(curr);
}

::std::string const&
publisher::wire_type(core::current const& curr) const
{
    throw errors::no_operation{
        curr.operation.identity,
        curr.operation.facet,
        "wire_type"
    };
}

core::object::type_list const&
publisher::wire_types(core::current const& curr) const
{
    throw errors::no_operation{
        curr.operation.identity,
        curr.operation.facet,
        "wire_type"
    };
}

bool
publisher::__wire_dispatch(core::detail::dispatch_request const& disp,
        core::current const& curr,
        dispatch_seen_list& seen, bool)
{
    if (!object::__wire_dispatch(disp, curr, seen, false)) {
        // Send to subscribers
        pimpl_->dispatch(disp, curr);
    }
    return true;
}

void
publisher::add_bus(core::identity const& id)
{
    pimpl_->add_bus(id);
}

void
publisher::remove_bus(core::identity const& id)
{
    pimpl_->remove_bus(id);
}

bool
publisher::bus_exists(core::identity const& id) const
{
    return pimpl_->bus_exists(id);
}

void
publisher::add_subscriber(core::identity const& bus_id, core::object_prx obj,
        core::current const& curr)
{
    pimpl_->add_subscriber(bus_id, obj, curr);
}

void
publisher::remove_subscriber(core::identity const& bus_id, core::object_prx obj,
        core::current const& curr)
{
    pimpl_->remove_subscriber(bus_id, obj, curr);
}

}  /* namespace svc */
}  /* namespace wire */
