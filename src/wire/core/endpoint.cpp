/*
 * endpoint.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: zmij
 */

#include <wire/core/endpoint.hpp>
#include <wire/core/grammar/endpoint_parse.hpp>
#include <wire/asio_config.hpp>
#include <wire/util/enumerate_interfaces.hpp>

#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>

#include <iostream>

namespace wire {
namespace core {
namespace detail {
static_assert(
    encoding::detail::wire_type< tcp_endpoint_data >::value == encoding::detail::STRUCT,
    "Wire type for tcp endpoint data is STRUCT");

static_assert(
    encoding::detail::wire_type< ssl_endpoint_data >::value == encoding::detail::STRUCT,
    "Wire type for ssl endpoint data is STRUCT");

static_assert(
    encoding::detail::wire_type< udp_endpoint_data >::value == encoding::detail::STRUCT,
    "Wire type for udp endpoint data is STRUCT");

static_assert(
    encoding::detail::wire_type< socket_endpoint_data >::value == encoding::detail::STRUCT,
    "Wire type for socket endpoint data is STRUCT");
}  // namespace detail

static_assert(
    encoding::detail::wire_type< endpoint >::value == encoding::detail::STRUCT,
    "Wire type for endpoint data is STRUCT");


namespace {
    const std::map< transport_type, std::string > TRANSPORT_TYPE_TO_STRING {
        { transport_type::empty, "empty" },
        { transport_type::tcp, "tcp" },
        { transport_type::ssl, "ssl" },
        { transport_type::udp, "udp" },
        { transport_type::socket, "socket" },
    }; // TRANSPORT_TYPE_TO_STRING
    const std::map< std::string, transport_type > STRING_TO_TRANSPORT_TYPE {
        { "empty", transport_type::empty },
        { "tcp", transport_type::tcp },
        { "ssl", transport_type::ssl },
        { "udp", transport_type::udp },
        { "socket", transport_type::socket },
    }; // STRING_TO_TRANSPORT_TYPE
} // namespace

// Generated output operator
std::ostream&
operator << (std::ostream& out, transport_type val)
{
    std::ostream::sentry s (out);
    if (s) {
        auto f = TRANSPORT_TYPE_TO_STRING.find(val);
        if (f != TRANSPORT_TYPE_TO_STRING.end()) {
            out << f->second;
        } else {
            out << "Unknown class " << (int)val;
        }
    }
    return out;
}
// Generated input operator
std::istream&
operator >> (std::istream& in, transport_type& val)
{
    std::istream::sentry s (in);
    if (s) {
        std::string name;
        if (in >> name) {
            auto f = STRING_TO_TRANSPORT_TYPE.find(name);
            if (f != STRING_TO_TRANSPORT_TYPE.end()) {
                val = f->second;
            } else {
                in.setstate(std::ios_base::failbit);
            }
        }
    }
    return in;
}

namespace detail {

namespace {

asio_config::address meta_address_v4{ asio_config::address_v4::from_string("0.0.0.0") };
asio_config::address meta_address_v6{ asio_config::address_v6::from_string("::") };

}  /* namespace  */

void
inet_endpoint_data::check() const
{
    if (host.empty()) {
        throw errors::logic_error("Empty host in an endpoint");
    }
    if (port == 0) {
        throw errors::logic_error("Port is not set in an endpoint");
    }
}

void
inet_endpoint_data::check(transport_type t) const
{
    if (host.empty()) {
        throw errors::logic_error("Empty host in ", t, " endpoint");
    }
    if (port == 0) {
        throw errors::logic_error("Port is not set in ", t ," endpoint");
    }
}

bool
inet_endpoint_data::is_meta_address() const
{
    asio_config::address addr{ asio_config::address::from_string(host) };
    if (addr.is_v4()) {
        return addr == meta_address_v4;
    } else {
        return addr == meta_address_v6;
    }
}

//----------------------------------------------------------------------------
void
socket_endpoint_data::check(transport_type t) const
{
    if (path.empty()) {
        throw errors::logic_error("Empty socket path in ", t, " endpoint");
    }
}

std::ostream&
operator << (std::ostream& os, empty_endpoint const& val)
{
    return os;
}


std::ostream&
operator << (std::ostream& os, inet_endpoint_data const& val)
{
    std::ostream::sentry s (os);
    if (s) {
        os << val.host << ':' << val.port;
    }
    return os;
}

std::ostream&
operator << (std::ostream& os, socket_endpoint_data const& val)
{
    std::ostream::sentry s (os);
    if (s) {
        os << val.path;
    }
    return os;
}

::std::size_t
hash_value(inet_endpoint_data const& val)
{
    return ::std::hash<::std::string>()(val.host) ^
            (::std::hash<uint16_t>()(val.port) << 1);
}

::std::size_t
hash_value(socket_endpoint_data const& val)
{
    return ::std::hash<::std::string>()(val.path);
}

}  // namespace detail

void
endpoint::swap(endpoint& rhs)
{
    using std::swap;
    swap(endpoint_data_, rhs.endpoint_data_);
}

endpoint&
endpoint::operator =(endpoint const& rhs)
{
    endpoint(rhs).swap(*this);
    return *this;
}

endpoint&
endpoint::operator =(endpoint&& rhs)
{
    endpoint(std::move(rhs)).swap(*this);
    return *this;
}

namespace detail {
struct endpoint_data_check : boost::static_visitor<> {
    transport_type expected_type;

    endpoint_data_check( transport_type e) : expected_type{e} {}
    void
    operator()( empty_endpoint const& data ) const
    {
        throw errors::logic_error( "Empty endpoint is not connectable" );
    }
    void
    operator()( inet_endpoint_data const& data ) const
    {
        data.check(expected_type);
    }
    void
    operator()( socket_endpoint_data const& data ) const
    {
        data.check(expected_type);
    }
};

struct collect_published_endpoints : boost::static_visitor<> {
    endpoint_list& endpoints;

    collect_published_endpoints(endpoint_list& eps) : endpoints{eps} {}

    void
    operator()( empty_endpoint const& data ) const
    {
        throw errors::logic_error( "Empty endpoint is not connectable" );
    }
    template<typename Endpoint>
    void
    check_meta_address(Endpoint const& data,
            endpoint (*func)(::std::string const&, uint16_t)) const
    {
        if (data.is_meta_address()) {
            asio_config::address addr{ asio_config::address::from_string(data.host) };
            util::get_interface_options opts =
                util::get_interface_options::loopback | util::get_interface_options::regular;
            if (addr.is_v4()) {
                opts = opts | util::get_interface_options::ip4;
            } else {
                opts = opts | util::get_interface_options::ip6;
            }
            auto addresses = util::get_local_interfaces(opts);
            for (auto const& addr : addresses) {
                endpoints.emplace_back( func( addr.to_string(), data.port ) );
            }
        } else {
            endpoints.emplace_back( data );
        }
    }
    void
    operator()( tcp_endpoint_data const& data ) const
    {
        check_meta_address(data, endpoint::tcp);
    }
    void
    operator()( ssl_endpoint_data const& data ) const
    {
        check_meta_address(data, endpoint::ssl);
    }
    void
    operator()( udp_endpoint_data const& data ) const
    {
        check_meta_address(data, endpoint::udp);
    }
    void
    operator()( socket_endpoint_data const& data ) const
    {
        endpoints.emplace_back( data );
    }
};

}  // namespace detail

void
endpoint::check(transport_type expected) const
{
    if (transport() != expected) {
        throw errors::logic_error("Invalid endpoint transport type ",
                transport(),
                " for ", expected, " transport");
    }
    boost::apply_visitor(detail::endpoint_data_check{ expected }, endpoint_data_);
}

void
endpoint::published_endpoints(endpoint_list& eps) const
{
    boost::apply_visitor(detail::collect_published_endpoints{ eps }, endpoint_data_ );
}

endpoint
endpoint::tcp(std::string const& host, uint16_t port)
{
    return endpoint{ detail::tcp_endpoint_data{ host, port } };
}

endpoint
endpoint::ssl(std::string const& host, uint16_t port)
{
    return endpoint{ detail::ssl_endpoint_data{ host, port } };
}
endpoint
endpoint::udp(std::string const& host, uint16_t port)
{
    return endpoint{ detail::udp_endpoint_data{ host, port } };
}
endpoint
endpoint::socket(std::string const& path)
{
    return endpoint{ detail::socket_endpoint_data{ path } };
}

std::ostream&
operator << (std::ostream& os, endpoint const& val)
{
    std::ostream::sentry s(os);
    if (s) {
        os << val.transport() << "://" << val.data();
    }
    return os;
}

std::istream&
operator >> (std::istream& is, endpoint& val)
{
    using istreambuf_iterator = std::istreambuf_iterator<char>;
    using stream_iterator = boost::spirit::multi_pass< istreambuf_iterator >;
    using endpoint_grammar = grammar::parse::endpoint_grammar< stream_iterator >;
    static endpoint_grammar endpoint_parser;

    std::istream::sentry s(is);
    if (s) {
        stream_iterator in = stream_iterator(istreambuf_iterator(is));
        stream_iterator eos = stream_iterator(istreambuf_iterator());
        endpoint tmp;
        if (boost::spirit::qi::parse(in, eos, endpoint_parser, tmp)) {
            tmp.swap(val);
        } else {
            is.setstate(std::ios_base::failbit);
        }
    }
    return is;
}

template < typename EndpointContainer >
::std::ostream&
write_endpoints (::std::ostream& os, EndpointContainer const& val)
{
    ::std::ostream::sentry s(os);
    if (s) {
        for (auto ep = val.begin(); ep != val.end(); ++ep) {
            if (ep != val.begin())
                os << ",";
            os << *ep;
        }
    }
    return os;
}


::std::ostream&
operator << (::std::ostream& os, endpoint_list const& val)
{
    return write_endpoints(os, val);
}

::std::ostream&
operator << (::std::ostream& os, endpoint_set const& val)
{
    return write_endpoints(os, val);
}

template < typename EndpointContainer >
::std::istream&
parse_endpoints (::std::istream& is, EndpointContainer& val)
{
    using istreambuf_iterator = std::istreambuf_iterator<char>;
    using stream_iterator = boost::spirit::multi_pass< istreambuf_iterator >;
    using endpoints_grammar = grammar::parse::endpoints_grammar< stream_iterator, EndpointContainer >;
    static endpoints_grammar endpoints_parser;

    ::std::istream::sentry s(is);
    if (s) {
        stream_iterator in = stream_iterator(istreambuf_iterator(is));
        stream_iterator eos = stream_iterator(istreambuf_iterator());
        EndpointContainer tmp;
        if (boost::spirit::qi::parse(in, eos, endpoints_parser, tmp)) {
            tmp.swap(val);
        } else {
            is.setstate(::std::ios_base::failbit);
        }
    }
    return is;
}


::std::istream&
operator >> (::std::istream& is, endpoint_list& val)
{
    return parse_endpoints(is, val);
}

::std::istream&
operator >> (::std::istream& is, endpoint_set& val)
{
    return parse_endpoints(is, val);
}

endpoint
operator "" _wire_ep(char const* str, ::std::size_t sz)
{
    ::std::string literal{str, sz};
    ::std::istringstream is{literal};
    endpoint ep;
    if (!(is >> ep))
        throw ::std::runtime_error{"Invalid ::wire::core::endpoint literal " + literal};
    return ep;
}

namespace detail {
struct endpoint_hash_visitor : boost::static_visitor<::std::size_t> {
    template < typename E >
    ::std::size_t
    operator()( E const& data ) const
    {
        return hash_value(data);
    }
};
}  // namespace detail

::std::size_t
hash(endpoint const& val)
{
    return ::boost::apply_visitor(detail::endpoint_hash_visitor{}, val.data());
}

::std::size_t
hash(endpoint_list const& eps)
{
    ::std::size_t h{0};

    for (auto const& ep : eps) {
        h = (h << 1) ^ hash(ep);
    }

    return h;
}

template class endpoint_rotation< endpoint_list >;
template class endpoint_rotation< endpoint_set >;

}  // namespace core
}  // namespace wire
