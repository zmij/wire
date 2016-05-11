/*
 * invokation.hpp
 *
 *  Created on: 11 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_CORE_INVOKATION_HPP_
#define WIRE_CORE_INVOKATION_HPP_

#include <wire/core/connection.hpp>
#include <wire/core/reference.hpp>
#include <wire/util/function_traits.hpp>

namespace wire {
namespace core {

namespace detail {

template < typename ... Args >
struct invokation_args_type {
    enum { arity = sizeof ... (Args) };
    using args_tuple_type   = ::std::tuple< Args ... >;

    template < ::std::size_t N >
    struct arg {
        using type          = typename ::std::tuple_element< N, args_tuple_type >::type;
    };

    using last_arg          = typename arg< arity - 1 >::type;
    using before_last_arg   = typename arg< arity - 2 >::type;
};

template < bool enoughArgs, typename ... Args >
struct is_sync_dispatch_impl : ::std::true_type {};

template < typename ... Args >
struct is_sync_dispatch_impl<true, Args ...>
    : ::std::conditional<
            ::std::is_same<
                 typename invokation_args_type< Args... >::before_last_arg,
                 functional::exception_callback >::value &&
            ::std::is_same<
                 typename invokation_args_type< Args... >::last_arg,
                 current const& >::value,
            ::std::false_type,
            ::std::true_type
         >::type {};


template < typename ... T >
struct is_sync_dispatch;

template < typename Interface, typename Return, typename ... Args >
struct is_sync_dispatch< Return(Interface::*)(Args ...) >
    : is_sync_dispatch_impl< sizeof ... (Args) >= 3, Args ... > {};

template < typename Interface, typename Return, typename ... Args >
struct is_sync_dispatch< Return(Interface::*)(Args ...) const>
    : is_sync_dispatch_impl< sizeof ... (Args) >= 3, Args ... > {};

enum class invokation_type {
    void_sync,
    nonvoid_sync,
    void_async
};

template < bool isVoid, bool isSync >
struct invokation_selector;

template < invokation_type T >
using invokation_constant = ::std::integral_constant< invokation_type, T >;

template <>
struct invokation_selector<true, true> :
    invokation_constant< invokation_type::void_sync >{};
template <>
struct invokation_selector<false, true> :
    invokation_constant< invokation_type::nonvoid_sync >{};
template <>
struct invokation_selector<true, false> :
    invokation_constant< invokation_type::void_async >{};

template < typename Handler, typename Member,
        typename IndexTuple, typename ... Args >
struct local_invokation;

template < typename Handler, typename Member,
            size_t ... Indexes, typename ... Args >
struct local_invokation< Handler, Member,
            util::indexes_tuple< Indexes ... >, Args ... > {
    using invocation_args =
            ::std::tuple<
                  ::std::reference_wrapper<
                        typename ::std::decay< Args >::type const > ... >;
    using response_hander   = Handler;
    using exception_handler = functional::exception_callback;
    using sent_handler      = functional::callback< bool >;
    using member_type       = Member;
    using member_traits     = util::function_traits< member_type >;
    using interface_type    = typename member_traits::class_type;
    using servant_ptr       = ::std::shared_ptr< interface_type >;

    static constexpr bool is_void
                            = ::std::is_same< typename member_traits::result_type, void >::value;
    static constexpr bool is_sync = is_sync_dispatch< member_type >::value;

    reference const&        ref;
    member_type             member;

    ::std::string const&    op;
    context_type const&     ctx;
    invocation_args         args;

    response_hander         response;
    exception_handler       exception;
    sent_handler            sent;

    void
    operator()(bool) const
    {
        invokation_sent();
        object_ptr obj; // Get local servant from connector
        if (!obj)
            invokation_error(::std::make_exception_ptr(errors::no_object{ref.object_id()}));
        servant_ptr srv = ::std::dynamic_pointer_cast< interface_type >(obj);
        if (!srv)
            invokation_error(::std::make_exception_ptr(errors::no_object{ref.object_id()}));
        current curr{{}, ctx};
        invoke(srv, curr, invokation_selector< is_void, is_sync >{});
    }

    void
    invoke( servant_ptr srv, current const& curr,
            invokation_constant< invokation_type::void_sync > const& ) const
    {
        try {
            ((*srv).*member)(::std::get< Indexes >(args).get() ..., curr);
            response();
        } catch (...) {
            invokation_error(::std::current_exception());
        }
    }

    void
    invoke( servant_ptr srv, current const& curr,
            invokation_constant< invokation_type::nonvoid_sync > const& ) const
    {
        try {
            response(((*srv).*member)(::std::get< Indexes >(args).get() ..., curr));
        } catch (...) {
            invokation_error(::std::current_exception());
        }
    }

    void
    invoke( servant_ptr srv, current const& curr,
            invokation_constant< invokation_type::void_async > const&) const
    {
        try {
            ((*srv).*member)(::std::get< Indexes >(args).get() ...,
                    response, exception, curr);
        } catch (...) {
            invokation_error(::std::current_exception());
        }
    }

    void
    invokation_sent() const
    {
        if (sent) {
            try {
                sent(true);
            } catch (...) {}
        }
    }
    void
    invokation_error(::std::exception_ptr ex) const
    {
        if (exception) {
            try {
                exception(ex);
            } catch (...) {
            }
        }
    }
};

template < typename Handler, typename IndexTuple, typename ... Args >
struct remote_invokation;

template < typename Handler, size_t ... Indexes, typename ... Args >
struct remote_invokation< Handler,
                util::indexes_tuple< Indexes ... >, Args ... > {
    using invocation_args =
            ::std::tuple<
                  ::std::reference_wrapper<
                        typename ::std::decay< Args >::type const > ... >;
    using response_hander   = Handler;
    using exception_handler = functional::exception_callback;
    using sent_handler      = functional::callback< bool >;

    reference const&        ref;

    ::std::string const&    op;
    context_type const&     ctx;
    invocation_args         args;

    response_hander         response;
    exception_handler       exception;
    sent_handler            sent;

    void
    operator()(bool run_sync) const
    {
        ref.get_connection()->invoke(ref.object_id(), op, ctx, run_sync,
                response, exception, sent, ::std::get< Indexes >(args).get() ...);
    }
};

}  /* namespace detail */

template < typename Handler, typename Member, typename ... Args>
functional::invocation_function
make_invocation(reference const&        ref,
        ::std::string const&            op,
        context_type const&             ctx,
        Member                          member,
        Handler                         response,
        functional::exception_callback  exception,
        functional::callback< bool >    sent,
        Args const& ...                 args)
{
    using index_type = typename util::index_builder< sizeof ... (Args) >::type;
    using remote_invokation = detail::remote_invokation< Handler, index_type, Args ... >;
    using local_invokation = detail::local_invokation< Handler, Member, index_type, Args ... >;
    if (ref.is_local()) {
        return local_invokation {
            ref, member, op, ctx,
            { ::std::cref(args)... },
            response, exception, sent
        };
    } else {
        return remote_invokation {
            ref, op, ctx,
            { ::std::cref(args)... },
            response, exception, sent
        };
    }
}

}  /* namespace core */
}  /* namespace wire */



#endif /* WIRE_CORE_INVOKATION_HPP_ */
