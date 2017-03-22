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
    using endpoint_cache_item       = util::timed_cache< endpoint_rotation_type, 10 >;
    using hash_value_type           = ::std::size_t;
    using endpoint_cache            = ::tbb::concurrent_hash_map<hash_value_type, endpoint_cache_item>;
    using endpoint_accessor         = endpoint_cache::accessor;
    using endpoint_const_accessor   = endpoint_cache::const_accessor;
    using shared_count              = ::std::shared_ptr< ::std::size_t >;

    connector_weak_ptr          connector_;

    mutex_type                  locator_mtx_;

    reference_data              locator_ref_;

    locator_prx                 locator_;
    locator_registry_prx        locator_reg_;

    endpoint_cache              endpoints_;

    void
    set_owner(connector_ptr c)
    {
        connector_ = c;
        auto const& options = c->options();
        locator_ref_ = options.locator_ref;
    }

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
        using functional::report_exception;

        auto cnctr = get_connector();
        locator_prx loc;
        {
            lock_guard lock{locator_mtx_};
            loc = locator_;
        }

        if (locator_ref_.object_id.empty()) {
            auto const& options = cnctr->options();
            locator_ref_ = options.locator_ref;
        }

        if (!loc && !locator_ref_.object_id.empty()) {
            #if DEBUG_OUTPUT >= 1
            ::std::cerr <<::getpid() << " Connecting to locator "
                    << locator_ref_ << "\n";
            #endif
            try {
                if (locator_ref_.endpoints.empty())
                    throw errors::runtime_error{ "Locator reference must have endpoints" };
                object_prx prx = cnctr->make_proxy(locator_ref_);
                auto opts = prx->wire_invocation_options();
                if (run_sync)
                    opts.flags |= invocation_flags::sync;
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
                        ::std::cerr <<::getpid() << " Exception in checked_cast<locator_proxy>\n";
                        #endif
                        report_exception(exception, ex);
                    }, nullptr, ctx, opts
                );
            } catch (...) {
                report_exception(exception, ::std::current_exception());
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
        using functional::report_exception;
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
                        ::std::cerr <<::getpid() << " Try to obtain locator registry proxy from locator "
                                << *loc << "\n";
                        #endif
                        auto opts = loc->wire_invocation_options();
                        if (run_sync)
                            opts.flags |= invocation_flags::sync;
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
                            }, exception, nullptr, ctx, opts);
                    } else {
                        try {
                            result(locator_reg_);
                        } catch (...) {}
                    }
                },
                #if DEBUG_OUTPUT >= 1
                [exception](::std::exception_ptr ex)
                {
                    ::std::cerr <<::getpid() << " Exception in get_locator_registry\n";
                    report_exception(exception, ex);
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
        locator_ref_    = loc->wire_get_reference()->data();
        locator_        = loc;
    }

    // TODO Make the member static and pass connector_ptr as argument
    static void
    get_connection(
            connector_ptr                       connector,
            endpoint_rotation_ptr               ep_rot,
            functional::callback<connection_ptr> result,
            functional::exception_callback      exception,
            shared_count                        retries,
            bool                                run_sync)
    {
        using functional::report_exception;
        if (!connector) {
            report_exception(exception,
                    errors::connector_destroyed{"Connector was already destroyed"});
        }

        ++(*retries);
        connector->get_outgoing_connection_async(
                ep_rot->next(), result,
                [connector, ep_rot, result, exception, retries, run_sync](::std::exception_ptr ex)
                {
                    // Check retry count
                    if (*retries < ep_rot->size()) {
                        // Rethrow the exception
                        try {
                            ::std::rethrow_exception(ex);
                        } catch (errors::connection_refused const& ex) {
                            // Retry if the exception is connection_refused
                            // and retry count doesn't exceed retry count
                            get_connection(connector, ep_rot, result,
                                    exception, retries, run_sync);
                        } catch (...) {
                            // Other exceptions are not retryable at this level
                            report_exception(exception, ::std::current_exception());
                        }
                    }
                },
                run_sync);
    }
    void
    get_connection(endpoint_rotation_ptr        ep_rot,
            functional::callback<connection_ptr> result,
            functional::exception_callback      exception,
            bool                                run_sync)

    {
        auto retries = ::std::make_shared<::std::size_t>(0);
        get_connection(connector_.lock(), ep_rot, result, exception, retries, run_sync);
    }
    bool
    get_connection(endpoint_const_accessor const& eps,
            functional::callback<connection_ptr> result,
            functional::exception_callback      exception,
            bool                                run_sync )
    {
        if (!eps.empty() && eps->second && !eps->second.stale()) {
            auto retries = ::std::make_shared<::std::size_t>(0);
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
        using functional::report_exception;
        if (!cache_lookup(ref, result, exception, run_sync)) {
            // Don't check endpoints - they are checked by lookup function
            get_locator_async(
            [this, ref, result, exception, run_sync](locator_prx loc)
            {
                if (!loc) {
                    report_exception( exception, errors::no_locator{} );
                    return;
                }
                auto opts = loc->wire_invocation_options();
                if (run_sync)
                    opts.flags |= invocation_flags::sync;
                if (ref.adapter.is_initialized()) {
                    loc->find_adapter_async(ref.adapter.get(),
                    [this, ref, result, exception, run_sync](object_prx obj)
                    {
                        auto const& objref = obj->wire_get_reference()->data();
                        auto eps = make_enpoint_rotation(objref);
                        cache_store(ref, eps);
                        get_connection(eps, result, exception, run_sync);
                    },
                    exception, nullptr, no_context, opts);
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
                                report_exception( exception, object_not_found{ ref.object_id } );
                            }
                        } else {
                            auto eps = make_enpoint_rotation(objref);
                            cache_store(ref, eps);
                            get_connection(eps, result, exception, run_sync);
                        }
                    }, exception, nullptr, no_context, opts);
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

reference_resolver::reference_resolver(connector_ptr cnctr, reference_data loc_ref)
    : pimpl_{ util::make_unique<impl>() }
{
    pimpl_->connector_      = cnctr;
    pimpl_->locator_ref_    = loc_ref;
}

reference_resolver::reference_resolver(reference_resolver&& rhs)
    : pimpl_{ ::std::move(rhs.pimpl_) }
{
}


reference_resolver::~reference_resolver() = default;

void
reference_resolver::set_owner(connector_ptr c)
{
    pimpl_->set_owner(c);
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
