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

#include <sstream>
#include <unordered_map>

#include <boost/thread/shared_mutex.hpp>

#if DEBUG_OUTPUT >= 1
#include <iostream>
#endif

namespace wire {
namespace svc {

struct publisher::impl {
    struct subscribers : ::std::enable_shared_from_this<subscribers> {
        using subscriber_map    = ::std::unordered_map<
                                    core::reference_data,
                                    core::object_prx>;
        using mutex_type        = ::boost::shared_mutex;
        using read_lock         = ::boost::shared_lock<mutex_type>;
        using write_lock        = ::boost::unique_lock<mutex_type>;
        using upgradable_lock   = ::boost::upgrade_lock<mutex_type>;
        using upgrade_lock      = ::boost::upgrade_to_unique_lock<mutex_type>;

        void
        dispatch(core::detail::dispatch_request const& disp,
                core::current const& curr) const
        {
            auto cncntr = disp.buffer->get_connector();
            auto _this = shared_from_this();
            // Copy encaps to out buffer
            read_lock lock{mutex_};
            for (auto const& s : subscribers_) {
                // Invoke op on all object proxies
                //s.second->
            }
        }
        void
        add_subscriber(core::object_prx obj)
        {
            upgradable_lock lock{mutex_};
            auto const& ref = obj->wire_get_reference()->data();
            auto f = subscribers_.find(ref);
            if (f == subscribers_.end()) {
                upgrade_lock write_lock{lock};
                subscribers_.emplace( ref, obj );
            }
        }

        void
        remove_subscriber(core::object_prx obj)
        {
            upgradable_lock lock{mutex_};
            auto const& ref = obj->wire_get_reference()->data();
            auto f = subscribers_.find(ref);
            if (f != subscribers_.end()) {
                upgrade_lock write_lock{lock};
                // TODO Second check
                subscribers_.erase(f);
            }
        }

        mutex_type mutable  mutex_;
        subscriber_map      subscribers_;
    };

    using subscribers_ptr = ::std::shared_ptr< subscribers >;
    using bus_subscribers = ::tbb::concurrent_hash_map<
                                    core::identity, subscribers_ptr >;

    void
    ping(core::current const& curr)
    {
        // Search for publisher id
        bus_subscribers::const_accessor f;
        if (!subscribers_.find(f, curr.operation.target.identity)) {
            // If none - throw no_object
            throw errors::no_object{
                curr.operation.target.identity,
                curr.operation.target.facet,
                "wire_ping"
            };
        }
    }

    void
    dispatch(core::detail::dispatch_request const& disp,
        core::current const& curr) const
    {
        bus_subscribers::const_accessor f;
        if (subscribers_.find(f, curr.operation.target.identity)) {
            // Send response to invoker
            disp.result(encoding::outgoing{disp.buffer->get_connector()});
            f->second->dispatch(disp, curr);
        } else {
            ::std::ostringstream os;
            os << curr.operation.operation;
            disp.exception( ::std::make_exception_ptr(
                errors::no_object{
                    curr.operation.target.identity,
                    curr.operation.target.facet,
                    os.str()
                }));
        }
    }

    void
    add_bus(core::identity const& id)
    {
        bus_subscribers::accessor f;
        if (subscribers_.insert(f, id)) {
            #if DEBUG_OUTPUT >= 1
            ::std::cerr << "Add bus " << id << "\n";
            #endif
            f->second = ::std::make_shared<subscribers>();
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
            f->second->add_subscriber(obj);
        } else {
            throw errors::no_object{
                curr.operation.target.identity,
                curr.operation.target.facet,
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
            f->second->remove_subscriber(obj);
        } else {
            throw errors::no_object{
                curr.operation.target.identity,
                curr.operation.target.facet,
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
        curr.operation.target.identity,
        curr.operation.target.facet,
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
        curr.operation.target.identity,
        curr.operation.target.facet,
        "wire_type"
    };
}

core::object::type_list const&
publisher::wire_types(core::current const& curr) const
{
    throw errors::no_operation{
        curr.operation.target.identity,
        curr.operation.target.facet,
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
