/*
 * client.hpp
 *
 *  Created on: Dec 20, 2017
 *      Author: zmij
 */

#ifndef WIRE_BUS_CLIENT_HPP_
#define WIRE_BUS_CLIENT_HPP_

#include <wire/future_config.hpp>
#include <wire/core/connector_fwd.hpp>
#include <wire/core/identity_fwd.hpp>
#include <wire/core/proxy_fwd.hpp>
#include <wire/core/reference_fwd.hpp>
#include <wire/core/functional.hpp>
#include <wire/core/invocation_options.hpp>

#include <wire/bus/bus_fwd.hpp>

#include <memory>

namespace wire {
namespace bus {

/**
 * A helper class to use wire bus service
 */
class client {
public:
    client();
    client(core::connector_ptr conn, core::reference_data const& ref);
    client(bus_registry_prx bus_reg);
    client(client const& rhs) = default;
    client(client&&) = default;

    client&
    operator = (client const&);
    client&
    operator = (client&&);

    void
    swap(client& rhs)
    {
        using ::std::swap;
        swap(pimpl_, rhs.pimpl_);
    }

    bool
    operator ! () const;
    operator bool() const;

    bus_prx
    get_bus(core::identity const& bus_id,
            core::invocation_options _opts = core::invocation_options::unspecified) const;
    void
    get_bus_async(core::identity const& bus_id,
            core::functional::callback<bus_prx> result,
            core::functional::exception_callback exception = nullptr,
            core::invocation_options _opts = core::invocation_options::unspecified
    ) const;
    template < template <typename> class _Promise = promise >
    auto
    get_bus_async(core::identity const& bus_id,
            core::invocation_options _opts = core::invocation_options::unspecified) const
    -> decltype(::std::declval<_Promise<bus_prx>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<bus_prx> >();

        get_bus_async(bus_id,
            [promise](bus_prx res)
            {
                promise->set_value(res);
            },
            [promise](::std::exception_ptr ex)
            {
                promise->set_exception(ex);
            }, _opts
        );

        return promise->get_future();
    }


    void
    subscribe(core::identity const& bus_id, core::object_prx prx,
            core::invocation_options _opts          = core::invocation_options::unspecified
        ) const;
    void
    subscribe_async(core::identity const& bus_id, core::object_prx prx,
            core::functional::void_callback,
            core::functional::exception_callback    = nullptr,
            core::invocation_options _opts          = core::invocation_options::unspecified
        ) const;
    template < template <typename> class _Promise = promise >
    auto
    subscribe_async(core::identity const& bus_id, core::object_prx prx,
            core::invocation_options _opts          = core::invocation_options::unspecified
        ) const
        -> decltype(::std::declval<_Promise<void>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<void> >();
        subscribe_async(bus_id, prx,
            [promise]()
            {
                promise->set_value();
            },
            [promise](::std::exception_ptr ex)
            {
                promise->set_exception(ex);
            }, _opts
        );

        return promise->get_future();
    }

    void
    unsubscribe(core::identity const& bus_id, core::object_prx prx,
            core::invocation_options _opts          = core::invocation_options::unspecified
        ) const;
    void
    unsubscribe_async(core::identity const& bus_id, core::object_prx prx,
            core::functional::void_callback,
            core::functional::exception_callback    = nullptr,
            core::invocation_options _opts          = core::invocation_options::unspecified
        ) const;
    template < template <typename> class _Promise = promise >
    auto
    unsubscribe_async(core::identity const& bus_id, core::object_prx prx,
            core::invocation_options _opts = core::invocation_options::unspecified) const
        -> decltype(::std::declval<_Promise<void>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<void> >();
        unsubscribe_async(bus_id, prx,
            [promise]()
            {
                promise->set_value();
            },
            [promise](::std::exception_ptr ex)
            {
                promise->set_exception(ex);
            }, _opts
        );

        return promise->get_future();
    }

    core::object_prx
    get_publisher(core::identity const& bus_id,
            core::invocation_options _opts = core::invocation_options::unspecified) const;
    void
    get_publisher_async(core::identity const& bus_id,
            core::functional::callback<core::object_prx> result,
            core::functional::exception_callback exception = nullptr,
            core::invocation_options _opts = core::invocation_options::unspecified
    ) const;

    template < template <typename> class _Promise = promise >
    auto
    get_publisher_async(core::identity const& bus_id,
            core::invocation_options _opts = core::invocation_options::unspecified) const
    -> decltype(::std::declval<_Promise<core::object_prx>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<core::object_prx> >();

        get_publisher_async(bus_id,
            [promise](core::object_prx res)
            {
                promise->set_value(res);
            },
            [promise](::std::exception_ptr ex)
            {
                promise->set_exception(ex);
            }, _opts
        );

        return promise->get_future();
    }

    template < typename ProxyType >
    ::std::shared_ptr< ProxyType >
    get_concrete_publisher(core::identity const& bus_id) const
    {
        auto obj = get_publisher(bus_id);
        return core::unchecked_cast< ProxyType >(obj);
    }

    template < typename ProxyType >
    void
    get_concrete_publisher_async(core::identity const& bus_id,
        core::functional::callback_set<::std::shared_ptr< ProxyType >> __resp,
        core::functional::exception_callback __exception) const
    {
        get_publisher_async(bus_id,
            [__resp]( core::object_prx obj)
            {
                __resp(core::unchecked_cast< ProxyType >(obj));
            }, __exception);
    }
private:
    struct impl;
    using pimpl = ::std::shared_ptr<impl>;
    pimpl pimpl_;
};

} /* namespace bus */
} /* namespace wire */



#endif /* WIRE_BUS_CLIENT_HPP_ */
