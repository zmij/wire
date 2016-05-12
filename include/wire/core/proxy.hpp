/*
 * proxy.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_PROXY_HPP_
#define WIRE_CORE_PROXY_HPP_

#include <wire/core/proxy_fwd.hpp>
#include <wire/core/reference.hpp>
#include <wire/core/connection_fwd.hpp>

#include <wire/core/context.hpp>
#include <wire/core/functional.hpp>
#include <wire/core/identity_fwd.hpp>

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
    object_proxy(reference_ptr ref);

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

    reference const&
    wire_get_reference() const
    { return *ref_; }

    connection_ptr
    wire_get_connection() const;

    template < typename T >
    ::std::shared_ptr<T>
    cast_to()
    {
        static_assert(::std::is_base_of< object_proxy, T >::value,
                "Can cast only to descendants of object_proxy type");
        return ::std::make_shared<T>(ref_);
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
            bool                            run_sync    = false
    );

    template < template< typename > class _Promise = ::std::promise >
    auto
    wire_is_a_async(::std::string const& type_id,
            context_type const& ctx = no_context, bool run_sync = false)
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
            nullptr, ctx, run_sync
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
            bool                            run_sync    = false
    );

    template< template< typename > class _Promise = ::std::promise >
    auto
    wire_ping_async(context_type const& ctx = no_context, bool run_sync = false)
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
            nullptr, ctx, run_sync
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
        bool                                        run_sync    = false
    );

    template < template< typename > class _Promise = ::std::promise >
    auto
    wire_type_async(context_type const& ctx = no_context, bool run_sync = false)
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
            nullptr, ctx, run_sync
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
        bool                                            run_sync    = false
    );

    template < template< typename > class _Promise = ::std::promise >
    auto
    wire_types_async(context_type const& ctx = no_context, bool run_sync = false)
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
            nullptr, ctx, run_sync
        );

        return promise->get_future();
    }
protected:
    object_proxy() {}
private:
    reference_ptr   ref_;
};

::std::ostream&
operator << (::std::ostream& os, object_proxy const& val);

template < typename Prx, typename ... Bases >
class proxy : public virtual Bases ... {

};

template < typename TargetPrx, typename SourcePrx>
::std::shared_ptr< TargetPrx >
unchecked_cast(::std::shared_ptr< SourcePrx > v)
{
    static_assert(::std::is_base_of<object_proxy, SourcePrx>::value,
            "Can cast only from instances of object_proxy");
    return v->template cast_to<TargetPrx>();
}

template < typename TargetPrx, typename SourcePrx >
::std::shared_ptr< TargetPrx >
checked_cast(::std::shared_ptr< SourcePrx > v)
{
    static_assert(::std::is_base_of<object_proxy, SourcePrx>::value,
            "Can cast only from instances of object_proxy");
    if (v->wire_is_a(TargetPrx::wire_static_type_id())) {
        return v->template cast_to<TargetPrx>();
    }
    return ::std::shared_ptr< TargetPrx >{};
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
        write(o, prx.wire_get_reference().data());
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
