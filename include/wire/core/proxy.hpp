/*
 * proxy.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_PROXY_HPP_
#define WIRE_CORE_PROXY_HPP_

#include <wire/core/identity_fwd.hpp>
#include <wire/core/proxy_fwd.hpp>
#include <wire/core/connection_fwd.hpp>
#include <wire/core/connector_fwd.hpp>
#include <wire/core/reference.hpp>

#include <wire/core/context.hpp>
#include <wire/core/functional.hpp>

#include <wire/encoding/wire_io.hpp>
#include <wire/encoding/buffers.hpp>

#include <future>
#include <functional>
#include <exception>
#include <vector>

namespace wire {
namespace core {

class object_proxy : public ::std::enable_shared_from_this< object_proxy > {
public:
    explicit
    object_proxy(reference_ptr ref, invocation_options const& ops = invocation_options{});

    virtual ~object_proxy() = default;

    bool
    operator == (object_proxy const&) const;
    bool
    operator != (object_proxy const& rhs) const
    { return !(*this == rhs); }
    bool
    operator < (object_proxy const&) const;

    void
    swap(object_proxy& rhs)
    {
        using ::std::swap;
        swap(ref_, rhs.ref_);
    }
public:
    identity const&
    wire_identity() const;

    reference_const_ptr
    wire_get_reference() const
    { return ref_; }

    invocation_options const&
    wire_invocation_options() const
    { return opts_; }

    connection_ptr
    wire_get_connection() const;
    connector_ptr
    wire_get_connector() const;

    template < typename T >
    ::std::shared_ptr<T>
    cast_to()
    {
        static_assert(::std::is_base_of< object_proxy, T >::value,
                "Can cast only to descendants of object_proxy type");
        return ::std::make_shared<T>(ref_, opts_);
    }

    static ::std::string const&
    wire_static_type_id();
public:

    bool
    wire_is_a(::std::string const&, context_type const& = no_context);

    void
    wire_is_a_async(
            ::std::string const&            type_id,
            functional::callback< bool >    response,
            functional::exception_callback  exception   = nullptr,
            functional::callback< bool >    sent        = nullptr,
            context_type const&                         = no_context,
            invocation_options              opts        = invocation_options::unspecified
    );

    template < template< typename > class _Promise = ::std::promise >
    auto
    wire_is_a_async(::std::string const&    type_id,
            context_type const&             ctx     = no_context,
            invocation_options const&       opts    = invocation_options::unspecified  )
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
            nullptr, ctx, opts
        );

        return promise->get_future();
    }

    void
    wire_ping(context_type const& = no_context);

    void
    wire_ping_async(
            functional::void_callback       response,
            functional::exception_callback  exception   = nullptr,
            functional::callback< bool >    sent        = nullptr,
            context_type const&                         = no_context,
            invocation_options              opts        = invocation_options::unspecified
    );

    template< template< typename > class _Promise = ::std::promise >
    auto
    wire_ping_async(context_type const&     ctx     = no_context,
            invocation_options const&       opts    = invocation_options::unspecified  )

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
            nullptr, ctx, opts
        );

        return promise->get_future();
    }

    ::std::string
    wire_type(context_type const& = no_context);

    void
    wire_type_async(
        functional::callback< ::std::string const& > response,
        functional::exception_callback              exception   = nullptr,
        functional::callback< bool >                sent        = nullptr,
        context_type const&                                     = no_context,
        invocation_options                          opts        = invocation_options::unspecified
    );

    template < template< typename > class _Promise = ::std::promise >
    auto
    wire_type_async(context_type const& ctx = no_context,
            invocation_options const&       opts    = invocation_options::unspecified  )
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
            nullptr, ctx, opts
        );

        return promise->get_future();
    }

    ::std::vector< ::std::string >
    wire_types(context_type const& = no_context);

    void
    wire_types_async(
        functional::callback< ::std::vector< ::std::string > const& > result,
        functional::exception_callback                  exception   = nullptr,
        functional::callback<bool>                      sent        = nullptr,
        context_type const&                                         = no_context,
        invocation_options                              opts        = invocation_options::unspecified
    );

    template < template< typename > class _Promise = ::std::promise >
    auto
    wire_types_async(context_type const&     ctx     = no_context,
            invocation_options const&        opts    = invocation_options::unspecified  )
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
            nullptr, ctx, opts
        );

        return promise->get_future();
    }

    object_prx
    wire_well_known_proxy() const;
    object_prx
    wire_with_identity(identity const& id) const;
    object_prx
    wire_with_endpoints(endpoint_list const& eps) const;
protected:
    object_proxy() {}
private:
    reference_ptr       ref_;
    invocation_options  opts_;
};

using object_seq = ::std::vector< object_prx >;

::std::ostream&
operator << (::std::ostream& os, object_proxy const& val);

template < typename Prx, typename ... Bases >
class proxy : public virtual Bases ... {
public:
    using proxy_type        = Prx;
    using proxy_ptr_type    = ::std::shared_ptr<proxy_type>;
public:
    proxy_ptr_type
    wire_one_way() const
    {
        return ::std::make_shared< proxy_type >(
            this->wire_get_reference(),
            this->wire_invocation_options() | invocation_flags::one_way);
    }

    proxy_ptr_type
    wire_timeout(invocation_options::timeout_type t) const
    {
        return ::std::make_shared< proxy_type >(
            this->wire_get_reference(),
            this->wire_invocation_options().with_timeout(t));
    }
};

template < typename TargetPrx, typename SourcePrx>
::std::shared_ptr< TargetPrx >
unchecked_cast(::std::shared_ptr< SourcePrx > v)
{
    static_assert(::std::is_base_of<object_proxy, SourcePrx>::value,
            "Can cast only from instances of object_proxy");
    if (v)
        return v->template cast_to<TargetPrx>();
    return ::std::shared_ptr< TargetPrx >{};
}

template < typename TargetPrx, typename SourcePrx >
void
checked_cast_async(::std::shared_ptr<SourcePrx>                 v,
        functional::callback< ::std::shared_ptr<TargetPrx> >    result,
        functional::exception_callback                          exception   = nullptr,
        functional::callback<bool>                              sent        = nullptr,
        context_type const&                                     ctx         = no_context,
        invocation_options const&                               opts        = invocation_options::unspecified)
{
    using result_prx = ::std::shared_ptr<TargetPrx>;
    static_assert(::std::is_base_of<object_proxy, SourcePrx>::value,
            "Can cast only from instances of object_proxy");
    if (v) {
        v->wire_is_a_async(
            TargetPrx::wire_static_type_id(),
            [v, result](bool is_a) {
                result_prx res = is_a ? v->template cast_to< TargetPrx >() : result_prx{};
                try {
                    result(res);
                } catch (...) {}
            },
            [exception](::std::exception_ptr ex) {
                if (exception) {
                    try {
                        exception(ex);
                    } catch (...) {}
                }
            }, sent, ctx, opts);
    } else {
        try {
            result(result_prx{});
        } catch (...) {}
    }
}

template < typename TargetPrx, typename SourcePrx,
    template <typename> class _Promise = ::std::promise >
auto
checked_cast_async(::std::shared_ptr<SourcePrx>                 v,
        context_type const& ctx           = no_context,
        invocation_options const& opts    = invocation_options::unspecified)
    -> decltype(::std::declval<_Promise< ::std::shared_ptr<TargetPrx>>>().get_future())
{
    using result_prx = ::std::shared_ptr<TargetPrx>;
    auto promise = ::std::make_shared<_Promise<result_prx>>();

    checked_cast_async<TargetPrx>(
        v,
        [promise](result_prx res)
        {
            promise->set_value(res);
        },
        [promise](::std::exception_ptr ex)
        {
            promise->set_exception(::std::move(ex));
        }, nullptr, ctx, opts
    );

    return promise->get_future();
}

template < typename TargetPrx, typename SourcePrx >
::std::shared_ptr< TargetPrx >
checked_cast(::std::shared_ptr< SourcePrx > v, context_type const& ctx = no_context)
{
    auto future = checked_cast_async<TargetPrx>(v, ctx,
            v->wire_invocation_options() | invocation_flags::sync);
    return future.get();
}

}  // namespace core
namespace encoding {
namespace detail {

template<>
struct is_proxy< core::object_proxy > : ::std::true_type {};

template < typename T >
struct writer_impl< T, PROXY > {
    using proxy_type         = typename polymorphic_type< T >::type;
    using proxy_ptr          = ::std::shared_ptr< proxy_type >;
    using proxy_weak_ptr    = ::std::weak_ptr< proxy_type >;

    template < typename OutputIterator >
    static void
    output(OutputIterator o, proxy_type const& prx)
    {
        using output_iterator_check = octet_output_iterator_concept< OutputIterator >;
        write(o, prx.wire_get_reference()->data());
    }

    template < typename OutputIterator >
    static void
    output(OutputIterator o, proxy_ptr prx)
    {
        using output_iterator_check = octet_output_iterator_concept< OutputIterator >;
        if (prx) {
            write(o, true);
            output(o, *prx);
        } else {
            write(o, false);
        }
    }

    template < typename OutputIterator >
    static void
    output(OutputIterator o, proxy_weak_ptr prx)
    {
        using output_iterator_check = octet_output_iterator_concept< OutputIterator >;
        output(o, prx.lock());
    }
};

template < typename T >
struct reader_impl < T, PROXY > {
    using proxy_type         = typename polymorphic_type< T >::type;
    using proxy_ptr          = ::std::shared_ptr< proxy_type >;
    using proxy_weak_ptr     = ::std::weak_ptr< proxy_type >;

    using input_iterator        = encoding::incoming::const_iterator;

    static void
    input(input_iterator& begin, input_iterator end, proxy_type& v)
    {
        core::reference_data ref;
        read(begin, end, ref);
        proxy_type tmp{ core::reference::create_reference(begin.get_connector(), ref) };
        v.swap(tmp);
    }

    static void
    input(input_iterator& begin, input_iterator end, proxy_ptr& v)
    {
        bool is_there{false};

        read(begin, end, is_there);
        if (is_there) {
            core::reference_data ref;
            read(begin, end, ref);
            auto tmp = ::std::make_shared< proxy_type >(
                    core::reference::create_reference(begin.get_connector(), ref));
            ::std::swap(v, tmp);
        } else {
            v.reset();
        }
    }

    static void
    input(input_iterator& begin, input_iterator end, proxy_weak_ptr v)
    {
        proxy_ptr prx;
        input(begin, end, prx);
        v = prx;
    }
};

}  /* namespace detail */
}  /* namespace encoding */
}  // namespace wire

#endif /* WIRE_CORE_PROXY_HPP_ */
