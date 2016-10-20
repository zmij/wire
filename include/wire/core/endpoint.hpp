/*
 * endpoint.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_ENDPOINT_HPP_
#define WIRE_CORE_ENDPOINT_HPP_

#include <string>
#include <iosfwd>
#include <unordered_set>
#include <boost/variant.hpp>

#include <wire/encoding/wire_io.hpp>
#include <wire/encoding/detail/variant_io.hpp>

namespace wire {
namespace core {

enum class transport_type {
    empty,
    tcp,
    ssl,
    udp,
    socket,
};

namespace detail {

struct empty_endpoint {
    bool
    operator == (empty_endpoint const&) const
    { return true; }
    bool
    operator != (empty_endpoint const&) const
    { return false; }
    bool
    operator < (empty_endpoint const&) const
    { return false; }
};

::std::ostream&
operator << (::std::ostream& os, empty_endpoint const& val);

inline ::std::size_t
hash_value(empty_endpoint const&)
{ return 0; }

template < typename OutputIterator >
void
wire_write(OutputIterator o, empty_endpoint const& v)
{
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator read, empty_endpoint& v)
{
}

struct inet_interface {
    enum class version_type { v4, v6 };
    ::std::string   name;
    version_type    version;

    void
    swap(inet_interface& rhs) noexcept
    {
        using ::std::swap;
        swap(name, rhs.name);
        swap(version, rhs.version);
    }
    ::std::string
    resolve_address() const;
};

::std::ostream&
operator << (::std::ostream& os, inet_interface const& val);
::std::istream&
operator >> (::std::istream& is, inet_interface& val);

struct inet_endpoint_data {
    ::std::string   host = "";
    uint16_t        port = 0;

    inet_endpoint_data()
        : host(), port(0)
    {
    }
    inet_endpoint_data(::std::string const& host, uint16_t port)
        : host(host), port(port)
    {
    }

    void
    swap(inet_endpoint_data& rhs) noexcept
    {
        using ::std::swap;
        swap(host, rhs.host);
        swap(port, rhs.port);
    }

    bool
    operator == (inet_endpoint_data const& rhs) const
    {
        return host == rhs.host && port == rhs.port;
    }
    bool
    operator != (inet_endpoint_data const& rhs) const
    {
        return !(*this == rhs);
    }
    bool
    operator < (inet_endpoint_data const& rhs) const
    {
        return host < rhs.host || (host == rhs.host && port < rhs.port);
    }

    void
    check() const;
    void
    check(transport_type expected) const;

    bool
    is_meta_address() const;
};

::std::ostream&
operator << (::std::ostream& os, inet_endpoint_data const& val);
::std::istream&
operator >> (::std::istream& is, inet_endpoint_data& val);

::std::size_t
hash_value(inet_endpoint_data const& val);

template < typename OutputIterator >
void
wire_write(OutputIterator o, inet_endpoint_data const& v)
{
    encoding::write(o, v.host);
    encoding::write(o, v.port);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator& end, inet_endpoint_data& v)
{
    encoding::read(begin, end, v.host);
    encoding::read(begin, end, v.port);
}

struct tcp_endpoint_data : inet_endpoint_data {
    tcp_endpoint_data() = default;
    tcp_endpoint_data( ::std::string const& host, uint16_t port)
        : inet_endpoint_data{host, port}
    {
    }
};

struct ssl_endpoint_data : inet_endpoint_data {
    ssl_endpoint_data() = default;
    ssl_endpoint_data( ::std::string const& host, uint16_t port)
        : inet_endpoint_data{host, port}
    {
    }
};

struct udp_endpoint_data : inet_endpoint_data {
    udp_endpoint_data() = default;
    udp_endpoint_data( ::std::string const& host, uint16_t port)
        : inet_endpoint_data{host, port}
    {
    }
};

struct socket_endpoint_data {
    ::std::string        path;

    bool
    operator == (socket_endpoint_data const& rhs) const
    {
        return path == rhs.path;
    }
    bool
    operator != (socket_endpoint_data const& rhs) const
    {
        return !(*this == rhs);
    }
    bool
    operator < (socket_endpoint_data const& rhs) const
    {
        return path < rhs.path;
    }

    void
    check(transport_type expected) const;
};

::std::ostream&
operator << (::std::ostream& os, socket_endpoint_data const& val);

::std::size_t
hash_value(socket_endpoint_data const&);

template < typename OutputIterator >
void
wire_write(OutputIterator o, socket_endpoint_data const& v)
{
    encoding::write(o, v.path);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, socket_endpoint_data& v)
{
    encoding::read(begin, end, v.path);
}

template < typename T >
struct endpoint_data_traits
    : ::std::integral_constant< transport_type, transport_type::empty > {};

template <>
struct endpoint_data_traits< tcp_endpoint_data >
    : ::std::integral_constant< transport_type, transport_type::tcp > {};

template <>
struct endpoint_data_traits< ssl_endpoint_data >
    : ::std::integral_constant< transport_type, transport_type::ssl > {};

template <>
struct endpoint_data_traits< udp_endpoint_data >
    : ::std::integral_constant< transport_type, transport_type::udp > {};

template <>
struct endpoint_data_traits< socket_endpoint_data >
    : ::std::integral_constant< transport_type, transport_type::socket > {};

}  // namespace detail

class endpoint;
using endpoint_list = ::std::vector<endpoint>;
using endpoint_set = ::std::unordered_set<endpoint>;

class endpoint {
public:
    using endpoint_data
        = boost::variant<
            detail::empty_endpoint,
            detail::tcp_endpoint_data,
            detail::ssl_endpoint_data,
            detail::udp_endpoint_data,
            detail::socket_endpoint_data
        >;
public:
    endpoint() : endpoint_data_{ detail::empty_endpoint{} } {}
    endpoint(endpoint const& rhs) : endpoint_data_{ rhs.endpoint_data_ } {}
    endpoint(endpoint&& rhs) : endpoint_data_{ ::std::move(rhs.endpoint_data_) } {}
    endpoint(endpoint_data const& data) : endpoint_data_{ data } {}
    endpoint(endpoint_data&& data) : endpoint_data_{ ::std::move(data) } {}

    void
    swap(endpoint& rhs);

    endpoint&
    operator = ( endpoint const& );
    endpoint&
    operator = ( endpoint&& );

    bool
    operator == (endpoint const& rhs) const
    {
        return endpoint_data_ == rhs.endpoint_data_;
    }
    bool
    operator != (endpoint const& rhs) const
    {
        return !(*this == rhs);
    }

    bool
    operator < (endpoint const& rhs) const
    {
        return endpoint_data_ < rhs.endpoint_data_;
    }
    bool
    operator <= (endpoint const& rhs) const
    {
        return endpoint_data_ <= rhs.endpoint_data_;
    }
    bool
    operator > (endpoint const& rhs) const
    {
        return endpoint_data_ > rhs.endpoint_data_;
    }
    bool
    operator >= (endpoint const& rhs) const
    {
        return endpoint_data_ >= rhs.endpoint_data_;
    }


    transport_type
    transport() const
    {
        return static_cast<transport_type>(endpoint_data_.which());
    }

    endpoint_data&
    data()
    { return endpoint_data_; }
    endpoint_data const&
    data() const
    { return endpoint_data_; }

    template < typename T >
    T&
    get()
    {
        return boost::get< T >(endpoint_data_);
    }
    template < typename T >
    T const&
    get() const
    {
        return boost::get< T >(endpoint_data_);
    }

    void
    check(transport_type expected) const;

    void
    published_endpoints(endpoint_list&) const;

    template < typename OutputIterator >
    void
    write( OutputIterator o ) const
    {
        encoding::write(o, endpoint_data_);
    }
    template < typename InputIterator >
    void
    read( InputIterator& begin, InputIterator end)
    {
        encoding::read(begin, end, endpoint_data_);
    }

    static endpoint
    tcp(::std::string const& host, uint16_t port);
    static endpoint
    ssl(::std::string const& host, uint16_t port);
    static endpoint
    udp(::std::string const& host, uint16_t port);
    static endpoint
    socket(::std::string const& path);
private:
    endpoint_data    endpoint_data_;
};

template < typename OutputIterator >
void
wire_write(OutputIterator o, endpoint const& v)
{
    v.write(o);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, endpoint& v)
{
    v.read(begin, end);
}

::std::size_t
hash(endpoint const&);

::std::size_t
hash(endpoint_list const&);

template < typename Endpoints >
class endpoint_rotation;

template < template < typename, typename... > class EndpointContainer >
class endpoint_rotation< EndpointContainer<endpoint> > {
public:
    using container_type    = EndpointContainer<endpoint>;
    using const_iterator    = typename container_type::const_iterator;
public:
    endpoint_rotation()
        : endpoints_{}, current_{endpoints_.end()} {}
    endpoint_rotation(container_type const& eps)
        : endpoints_{eps}, current_{endpoints_.end()} {}
    endpoint_rotation(container_type&& eps)
        : endpoints_{eps}, current_{endpoints_.end()} {}

    bool
    empty() const
    { return endpoints_.empty(); }

    endpoint
    next() const
    {
        if (endpoints_.empty())
            throw errors::runtime_error{"Endpoint list is empty"};
        if (current_ == endpoints_.end()) {
            current_ = endpoints_.begin();
        } else {
            ++current_;
            if (current_ == endpoints_.end())
                current_ = endpoints_.begin();
        }
        return *current_;
    }

    container_type const&
    endpoints() const
    { return endpoints_; }
private:
    container_type          endpoints_;
    const_iterator mutable  current_;
};

template < typename Endpoints >
::std::size_t
hash(endpoint_rotation<Endpoints> const& eps)
{
    return hash(eps.endpoints());
}

//----------------------------------------------------------------------------
::std::ostream&
operator << (::std::ostream& os, transport_type val);
::std::istream&
operator >> (::std::istream& in, transport_type& val);
::std::ostream&
operator << (::std::ostream& os, endpoint const& val);
::std::istream&
operator >> (::std::istream& is, endpoint& val);

::std::ostream&
operator << (::std::ostream& os, endpoint_list const& val);
::std::istream&
operator >> (::std::istream& is, endpoint_list& val);

::std::ostream&
operator << (::std::ostream& os, endpoint_set const& val);
::std::istream&
operator >> (::std::istream& is, endpoint_set& val);

endpoint
operator "" _wire_ep(char const*, ::std::size_t);

}  // namespace core
}  // namespace wire

using ::wire::core::operator ""_wire_ep;

namespace std {

template <>
struct hash< ::wire::core::endpoint > {
    using argument_type = ::wire::core::endpoint;
    using result_type   = ::std::size_t;
    result_type
    operator()(argument_type const& v) const
    {
        return ::wire::core::hash(v);
    }
};

template <>
struct hash< ::wire::core::endpoint_list > {
    using argument_type = ::wire::core::endpoint_list;
    using result_type   = ::std::size_t;
    result_type
    operator()(argument_type const& v) const
    {
        return ::wire::core::hash(v);
    }
};

}  // namespace std

namespace wire {
namespace core {

extern template class endpoint_rotation< endpoint_list >;
extern template class endpoint_rotation< endpoint_set >;

inline size_t
tbb_hasher(endpoint const& ep) {
    return hash(ep);
}

template< typename ContainerA, typename ContainerB >
endpoint_set
merge_endpoints(ContainerA const& a, ContainerB const& b )
{
    endpoint_set eps{ a.begin(), a.end() };
    ::std::copy(b.begin(), b.end(), ::std::inserter(eps, eps.begin()));
    return eps;
}

}  /* namespace core */
}  /* namespace wire */

#endif /* WIRE_CORE_ENDPOINT_HPP_ */
