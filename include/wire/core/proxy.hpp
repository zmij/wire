/*
 * proxy.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_PROXY_HPP_
#define WIRE_CORE_PROXY_HPP_

#include <wire/core/proxy_fwd.hpp>
#include <wire/core/callbacks.hpp>
#include <wire/core/context.hpp>

#include <future>
#include <functional>
#include <exception>
#include <vector>

namespace wire {
namespace core {

class object_proxy : public ::std::enable_shared_from_this< object_proxy > {
public:
    virtual ~object_proxy() = default;

    bool
    operator == (object_proxy const&) const;
    bool
    operator != (object_proxy const& rhs) const
    { return !(*this == rhs); }
    bool
    operator < (object_proxy const&) const;
public:

    bool
    wire_is_a(::std::string const&, context_type const& = no_context);

    void
    wire_is_a_async(
            ::std::string const&            type_id,
            callbacks::callback< bool >     response,
            callbacks::exception_callback   exception   = nullptr,
            callbacks::callback< bool >     sent        = nullptr,
            context_type const&                         = no_context
    );

    template < template< typename > class _Promise = ::std::promise >
    auto
    wire_is_a_async(::std::string const& type_id, context_type const& ctx = no_context)
        -> decltype(::std::declval<_Promise<bool>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<bool> >();

        wire_is_a_async(
            type_id,
            [promise](bool val)
            {
                promise->set_value(val);
            },
            [promise](::std::exception_ptr ex)
            {
                promise->set_exception(::std::move(ex));
            },
            nullptr, ctx
        );

        return promise->get_future();
    }

    void
    wire_ping(context_type const& = no_context);

    void
    wire_ping_async(
            callbacks::void_callback        response,
            callbacks::exception_callback   exception   = nullptr,
            callbacks::callback< bool >     sent        = nullptr,
            context_type const&                         = no_context
    );

    template< template< typename > class _Promise = ::std::promise >
    auto
    wire_ping_async(context_type const& ctx = no_context)
        -> decltype(::std::declval<_Promise<void>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<void> >();

        wire_ping_async(
            [promise]()
            {
                promise->set_value();
            },
            [promise](::std::exception_ptr ex)
            {
                promise->set_exception(::std::move(ex));
            },
            nullptr, ctx
        );

        return promise->get_future();
    }

    ::std::string
    wire_type(context_type const& = no_context);

    void
    wire_type_async(
        callbacks::callback< ::std::string const& >   response,
        callbacks::exception_callback               exception   = nullptr,
        callbacks::callback< bool >                 sent        = nullptr,
        context_type const&                                     = no_context
    );

    template < template< typename > class _Promise = ::std::promise >
    auto
    wire_type_async(context_type const& ctx = no_context)
        -> decltype(::std::declval<_Promise<::std::string>>().get_future())
    {
        auto promise = ::std::make_shared<_Promise<::std::string>>();

        wire_type_async(
            [promise](::std::string const& val)
            {
                promise->set_value(val);
            },
            [promise](::std::exception_ptr ex)
            {
                promise->set_exception(::std::move(ex));
            },
            nullptr, ctx
        );

        return promise->get_future();
    }

    ::std::vector< ::std::string >
    wire_types(context_type const& = no_context);

    void
    wire_types_async(
        callbacks::callback< ::std::vector< ::std::string > const& > result,
        callbacks::exception_callback                    exception  = nullptr,
        callbacks::callback<bool>                        sent       = nullptr,
        context_type const&                                         = no_context
    );

    template < template< typename > class _Promise = ::std::promise >
    auto
    wire_types_async(context_type const& ctx = no_context)
        -> decltype(::std::declval<_Promise<::std::vector< ::std::string >>>().get_future())
    {
        auto promise = ::std::make_shared<_Promise<::std::vector< ::std::string >>>();

        wire_types_async(
            [promise](::std::vector< ::std::string > const& val)
            {
                promise->set_value(val);
            },
            [promise](::std::exception_ptr ex)
            {
                promise->set_exception(::std::move(ex));
            },
            nullptr, ctx
        );

        return promise->get_future();
    }

};

}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_PROXY_HPP_ */
