/*
 * reference_resolver.hpp
 *
 *  Created on: Oct 3, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_DETAIL_REFERENCE_RESOLVER_HPP_
#define WIRE_CORE_DETAIL_REFERENCE_RESOLVER_HPP_

#include <wire/core/connector_fwd.hpp>
#include <wire/core/reference_fwd.hpp>
#include <wire/core/locator_fwd.hpp>
#include <wire/core/connection_fwd.hpp>
#include <wire/core/context.hpp>

#include <wire/core/functional.hpp>

namespace wire {
namespace core {
namespace detail {

class reference_resolver {
public:
    reference_resolver();
    ~reference_resolver();

    void
    set_owner(connector_ptr c);

    void
    get_locator_async(
        functional::callback<locator_prx>   result,
        functional::exception_callback      exception   = nullptr,
        context_type const&                             = no_context,
        bool                                run_sync    = false
    ) const;
    void
    set_locator(locator_prx);

    void
    get_locator_registry_async(
        functional::callback<locator_registry_prx> result,
        functional::exception_callback      exception   = nullptr,
        context_type const&                             = no_context,
        bool                                run_sync    = false
    ) const;

    void
    resolve_reference_async(reference_data const& ref,
        functional::callback<connection_ptr> result,
        functional::exception_callback      exception   = nullptr,
        bool                                run_sync    = false
    ) const;
private:
    struct impl;
    using pimpl = ::std::unique_ptr<impl>;
    pimpl pimpl_;
};

} /* namespace detail */
} /* namespace core */
} /* namespace wire */

#endif /* WIRE_CORE_DETAIL_REFERENCE_RESOLVER_HPP_ */
