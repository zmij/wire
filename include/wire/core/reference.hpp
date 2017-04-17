/*
 * reference.hpp
 *
 *  Created on: Apr 11, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_REFERENCE_HPP_
#define WIRE_CORE_REFERENCE_HPP_

#include <wire/asio_config.hpp>
#include <wire/future_config.hpp>

#include <wire/core/identity.hpp>
#include <wire/core/endpoint.hpp>

#include <wire/core/connection_fwd.hpp>
#include <wire/core/connector_fwd.hpp>
#include <wire/core/reference_fwd.hpp>
#include <wire/core/object_fwd.hpp>
#include <wire/core/adapter_fwd.hpp>
#include <wire/core/proxy_fwd.hpp>
#include <wire/core/locator_fwd.hpp>
#include <wire/core/functional.hpp>
#include <wire/core/detail/future_traits.hpp>

#include <wire/encoding/detail/optional_io.hpp>

namespace wire {
namespace core {

/**
 * Reference data structure.
 *
 * Stringified representation: object_id[facet]@adapter endpoints
 */
struct reference_data {
    identity                object_id;
    ::std::string           facet;
    optional_identity       adapter;
    endpoint_list           endpoints; // TODO Replace with endpoint_set

    locator_prx             locator;
    // TODO router proxy

    bool
    valid() const
    {
        if (object_id.empty())
            return false;
        if (endpoints.empty() && !adapter.is_initialized())
            return false;
        return true;
    }
};

template < typename OutputIterator >
void
wire_write(OutputIterator o, reference_data const& v)
{
    encoding::write(o, v.object_id, v.facet, v.adapter, v.endpoints);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, reference_data& v)
{
    encoding::read(begin, end, v.object_id, v.facet, v.adapter, v.endpoints);
}

bool
operator == (reference_data const& lhs, reference_data const& rhs);
bool
operator != (reference_data const& lhs, reference_data const& rhs);
bool
operator < (reference_data const& lhs, reference_data const& rhs);

::std::ostream&
operator << (::std::ostream& os, reference_data const& val);
::std::istream&
operator >> (::std::istream& is, reference_data& val);

reference_data
operator "" _wire_ref(char const*, ::std::size_t);

::std::size_t
id_facet_hash(reference_data const&);
::std::size_t
hash(reference_data const&);

/**
 * Class for a reference.
 */
class reference : public ::std::enable_shared_from_this<reference> {
public:
    using connection_callback = functional::callback< connection_ptr >;
    using local_servant         = ::std::pair<object_ptr, adapter_ptr>;
public:
    reference(connector_ptr cn, reference_data const& ref)
        : connector_{cn}, ref_{ref} {}

    virtual ~reference() = default;

    static reference_ptr
    create_reference(connector_ptr, reference_data const&);

    reference_data const&
    data() const
    { return ref_; }

    bool
    is_local() const;
    local_servant
    get_local_object() const;

    identity const&
    object_id() const
    { return ref_.object_id; }

    ::std::string const&
    facet() const
    { return ref_.facet; }

    asio_config::io_service_ptr
    io_service() const;

    connector_ptr
    get_connector() const;

    template < template < typename  > class _Promise = promise >
    connection_ptr
    get_connection() const
    {
        auto future = get_connection_async<_Promise>(
                detail::promise_want_io_throttle<_Promise< connection_ptr >>::value );
        return future.get();
    }

    template < template < typename  > class _Promise = promise >
    auto
    get_connection_async(bool sync = false) const
        -> decltype(::std::declval< _Promise< connection_ptr > >().get_future() )
    {
        auto promise = ::std::make_shared< _Promise< connection_ptr > >();
        get_connection_async(
            [promise](connection_ptr val)
            {
                promise->set_value(val);
            },
            [promise](::std::exception_ptr ex)
            {
                promise->set_exception(::std::move(ex));
            }, sync
        );

        return promise->get_future();
    }

    virtual void
    get_connection_async( connection_callback on_get,
            functional::exception_callback on_error,
            bool sync = false) const  = 0;

    locator_prx
    get_locator() const;

    void
    set_locator(locator_prx);
    void
    set_locator(reference_data const& loc_ref);
protected:
    template < typename T >
    ::std::shared_ptr<T>
    shared_this()
    {
        return ::std::static_pointer_cast<T>(shared_from_this());
    }
    template < typename T >
    ::std::shared_ptr<T const>
    shared_this() const
    {
        return ::std::static_pointer_cast<T const>(shared_from_this());
    }
private:
    connector_weak_ptr  connector_;
protected:
    reference_data              ref_;
    object_weak_ptr mutable     local_object_cache_;
    adapter_weak_ptr mutable    adapter_cache_;
};

/**
 * Fixed reference
 *
 * A reference with fixed endpoints.
 */
class fixed_reference : public reference {
public:
    fixed_reference(connector_ptr cn, reference_data const& ref);
    fixed_reference(fixed_reference const& rhs);
    fixed_reference(fixed_reference&& rhs);

    void
    get_connection_async( connection_callback on_get,
            functional::exception_callback on_error,
            bool sync = false) const override;
private:
    using mutex_type                        = ::std::mutex;
    using lock_guard                        = ::std::lock_guard<mutex_type>;
    mutex_type mutable                      mutex_;
    connection_weak_ptr mutable             connection_;
    endpoint_list::const_iterator mutable   current_;
};

/**
 * Reference to an object with a named adapter. Adapter can be a replica group,
 * therefore having multiple endpoints. The get_connection method will return
 * next endpoint every time it is invoked.
 */
class floating_reference : public reference {
public:
    floating_reference(connector_ptr cn, reference_data const& ref);
    floating_reference(floating_reference const& rhs);
    floating_reference(floating_reference&& rhs);

    void
    get_connection_async( connection_callback on_get,
            functional::exception_callback on_error,
            bool sync = false) const override;
};

class routed_reference : public reference {
};

inline ::std::size_t
tbb_hasher(reference_data const& ref)
{
    return hash(ref);
}

}  // namespace core
}  // namespace wire

namespace std {

template <>
struct hash<::wire::core::reference_data> {
    using argument_type = ::wire::core::reference_data;
    using result_type   = ::std::size_t;
    result_type
    operator()(argument_type const& v) const
    {
        return ::wire::core::hash(v);
    }
};

}  /* namespace std */

using ::wire::core::operator ""_wire_ref;

#endif /* WIRE_CORE_REFERENCE_HPP_ */
