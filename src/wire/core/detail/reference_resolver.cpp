/*
 * reference_resolver.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: zmij
 */

#include <wire/core/detail/reference_resolver.hpp>
#include <wire/core/detail/configuration_options.hpp>
#include <wire/core/connector.hpp>
#include <wire/core/reference.hpp>
#include <wire/core/proxy.hpp>
#include <wire/core/locator.hpp>

#include <wire/util/make_unique.hpp>
#include <wire/util/timed_cache.hpp>

#include <tbb/concurrent_hash_map.h>
#include <mutex>
#if DEBUG_OUTPUT >= 1
#include <iostream>
#endif

namespace wire {
namespace core {
namespace detail {

struct reference_resolver::impl {
    using mutex_type                = ::std::mutex;
    using lock_guard                = ::std::lock_guard<mutex_type>;
    using endpoint_rotation_type    = endpoint_rotation< endpoint_list >;
    using endpoint_rotation_ptr     = ::std::shared_ptr< endpoint_rotation_type >;
    using endpoint_cache_item       = util::timed_cache< endpoint_rotation_type >;
    using hash_value_type           = ::std::size_t;
    using endpoint_cache            = ::tbb::concurrent_hash_map<hash_value_type, endpoint_cache_item>;
    using endpoint_accessor         = endpoint_cache::accessor;
    using endpoint_const_accessor   = endpoint_cache::const_accessor;

    connector_weak_ptr          connector_;

    mutex_type                  locator_mtx_;
    locator_prx                 locator_;
    locator_registry_prx        locator_reg_;

    endpoint_cache              endpoints_;

    connector_ptr
    get_connector()
    {
        auto cnctr = connector_.lock();
        if (!cnctr) {
            throw errors::connector_destroyed{"Connector was already destroyed"};
        }
        return cnctr;
    }

    void
    get_locator_async(functional::callback< locator_prx >  result,
            functional::exception_callback                  exception,
            context_type const&                             ctx,
            bool                                            run_sync)
    {
        auto cnctr = get_connector();
        locator_prx loc;
        {
            lock_guard lock{locator_mtx_};
            loc = locator_;
        }

        auto const& options = cnctr->options();

        if (!loc && !options.locator_ref.object_id.empty()) {
            #if DEBUG_OUTPUT >= 1
            ::std::cerr << "Connecting to locator "
                    << options.locator_ref << "\n";
            #endif
            try {
                if (options.locator_ref.endpoints.empty())
                    throw errors::runtime_error{ "Locator reference must have endpoints" };
                object_prx prx = cnctr->make_proxy(options.locator_ref);
                checked_cast_async< locator_proxy >(
                    prx,
                    [this, result](locator_prx loc)
                    {
                        if (loc) {
                            set_locator(loc);
                        }
                        try {
                            result(loc);
                        } catch (...) {}
                    },
                    [exception](::std::exception_ptr ex)
                    {
                        #if DEBUG_OUTPUT >= 1
                        ::std::cerr << "Exception in checked_cast<locator_proxy>\n";
                        #endif
                        if (exception) {
                            try {
                                exception(ex);
                            } catch (...) {}
                        }
                    }, nullptr, ctx, run_sync
                );
            } catch (...) {
                if (exception) {
                    try {
                        exception(::std::current_exception());
                    } catch (...) {}
                }
            }
        } else {
            try {
                result(loc);
            } catch (...) {}
        }
    }

    void
    get_locator_registry_async(
            functional::callback< locator_registry_prx >    result,
            functional::exception_callback                  exception,
            context_type const&                             ctx,
            bool                                            run_sync)
    {
        locator_registry_prx reg;
        {
            lock_guard lock{locator_mtx_};
            reg = locator_reg_;
        }
        if (!reg) {
            get_locator_async(
                [this, result, exception, ctx, run_sync](locator_prx loc)
                {
                    if (loc) {
                        #if DEBUG_OUTPUT >= 1
                        ::std::cerr << "Try to obtain locator registry proxy from locator "
                                << *loc << "\n";
                        #endif
                        loc->get_registry_async(
                            [this, result](locator_registry_prx reg)
                            {
                                if (reg) {
                                    lock_guard lock{locator_mtx_};
                                    locator_reg_ = reg;
                                }
                                try {
                                    result(locator_reg_);
                                } catch (...) {}
                            }, exception, nullptr, ctx, run_sync);
                    } else {
                        try {
                            result(locator_reg_);
                        } catch (...) {}
                    }
                },
                #if DEBUG_OUTPUT >= 1
                [exception](::std::exception_ptr ex)
                {
                    ::std::cerr << "Exception in get_locator_registry\n";
                    exception(ex);
                },
                #else
                exception,
                #endif
                ctx, run_sync);
        } else {
            try {
                result(reg);
            } catch (...) {}
        }
    }

    void
    set_locator(locator_prx loc)
    {
        lock_guard lock{locator_mtx_};
        locator_ = loc;
    }

    void
    get_connection(endpoint_rotation_ptr ep_rot,
            functional::callback<connection_ptr> result,
            functional::exception_callback      exception,
            bool                                run_sync)
    {
        try {
            auto cnctr = get_connector();
            cnctr->get_outgoing_connection_async(
                    ep_rot->next(), result, exception, run_sync);
        } catch (...) {
            if (exception) {
                try {
                    exception(::std::current_exception());
                } catch (...) {}
            }
        }
    }
    bool
    get_connection(endpoint_const_accessor const& eps,
            functional::callback<connection_ptr> result,
            functional::exception_callback      exception,
            bool                                run_sync )
    {
        if (!eps.empty() && eps->second) {
            get_connection(eps->second, result, exception, run_sync);
            return true;
        }
        return false;
    }

    void
    cache_store(hash_value_type key, endpoint_rotation_ptr const& eps)
    {
        endpoint_accessor f;
        endpoints_.insert(f, key);
        f->second = eps;
    }
    void
    cache_store(reference_data const& ref, endpoint_rotation_ptr const& eps)
    {
        cache_store(hash(*eps), eps);
        cache_store(id_facet_hash(ref), eps);
        if (ref.adapter.is_initialized()) {
            cache_store(hash(ref.adapter.get()), eps);
        }
    }

    bool
    cache_lookup(reference_data const& ref,
            functional::callback<connection_ptr> result,
            functional::exception_callback      exception,
            bool                                run_sync)
    {
        endpoint_accessor eps;
        if (!ref.endpoints.empty()) {
            // Fixed endpoints list, return one
            auto eps_hash = hash(ref.endpoints);

            if (endpoints_.insert(eps, eps_hash) || !eps->second) {
                eps->second = endpoint_cache_item{ ref.endpoints };
            }
            return get_connection(eps, result, exception, run_sync);
        }

        // Find endpoints by object_id
        auto id_hash = id_facet_hash(ref);
        if (endpoints_.find(eps, id_hash) && get_connection(eps, result, exception, run_sync)) {
            return true;
        }

        if (ref.adapter.is_initialized()) {
            // Find endpoints by adapter id
            auto adapter_hash = hash(ref.adapter.get());
            if (endpoints_.find(eps, adapter_hash)
                    && get_connection(eps, result, exception, run_sync)) {
                // Cache to object id
                endpoint_accessor oid_eps;
                endpoints_.insert(oid_eps, id_hash);
                oid_eps->second = eps->second;
                return true;
            }
        }
        return false;
    }

    void
    resolve_reference_async(reference_data const& ref,
            functional::callback<connection_ptr> result,
            functional::exception_callback      exception,
            bool                                run_sync
        )
    {
        if (!cache_lookup(ref, result, exception, run_sync)) {
            // Don't check endpoints - they are checked by lookup function
            get_locator_async(
            [this, ref, result, exception, run_sync](locator_prx loc)
            {
                if (!loc) {
                    exception( ::std::make_exception_ptr( errors::no_locator{} ) );
                    return;
                }
                if (ref.adapter.is_initialized()) {
                    loc->find_adapter_async(ref.adapter.get(),
                    [this, ref, result, exception, run_sync](object_prx obj)
                    {
                        auto const& objref = obj->wire_get_reference()->data();
                        auto eps = make_enpoint_rotation(objref);
                        cache_store(ref, eps);
                        get_connection(eps, result, exception, run_sync);
                    },
                    exception, nullptr, no_context, run_sync);
                } else {
                    // Lookup by object id
                    loc->find_object_async(ref.object_id,
                    [this, ref, result, exception, run_sync](object_prx obj)
                    {
                        auto const& objref = obj->wire_get_reference()->data();
                        // Well-known object can have no endpoints initialized, only adapter set
                        if (objref.endpoints.empty()) {
                            // Resolve by adapter, if any
                            if (objref.adapter.is_initialized()) {
                                resolve_reference_async(objref, result, exception, run_sync);
                            } else {
                                // This is an error situation - locator returned
                                // a proxy containing only object id
                                exception(
                                    ::std::make_exception_ptr( object_not_found{ ref.object_id } ) );
                            }
                        } else {
                            auto eps = make_enpoint_rotation(objref);
                            cache_store(ref, eps);
                            get_connection(eps, result, exception, run_sync);
                        }
                    }, exception, nullptr, no_context, run_sync);
                }
            }, exception, no_context, run_sync);
        }
    }

    endpoint_rotation_ptr
    make_enpoint_rotation(
            const wire::core::reference_data& objref)
    {
        return ::std::make_shared<endpoint_rotation_type>(objref.endpoints);
    }
};

reference_resolver::reference_resolver()
    : pimpl_{ util::make_unique<impl>() }
{
}

reference_resolver::~reference_resolver() = default;

void
reference_resolver::set_owner(connector_ptr c)
{
    pimpl_->connector_ = c;
}

void
reference_resolver::set_locator(locator_prx loc)
{
    pimpl_->set_locator(loc);
}

void
reference_resolver::get_locator_async(functional::callback<locator_prx>   result,
        functional::exception_callback      exception,
        context_type const&                 ctx,
        bool                                run_sync) const
{
    pimpl_->get_locator_async(result, exception, ctx, run_sync);
}

void
reference_resolver::get_locator_registry_async(
    functional::callback<locator_registry_prx> result,
    functional::exception_callback      exception,
    context_type const&                 ctx,
    bool                                run_sync) const
{
    pimpl_->get_locator_registry_async(result, exception, ctx, run_sync);
}


void
reference_resolver::resolve_reference_async(reference_data const& ref,
        functional::callback<connection_ptr> result,
        functional::exception_callback      exception,
        bool                                run_sync
    ) const
{
    pimpl_->resolve_reference_async(ref, result, exception, run_sync);
}

} /* namespace detail */
} /* namespace core */
} /* namespace wire */
