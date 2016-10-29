/*
 * enpoint_parse.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_GRAMMAR_ENDPOINT_PARSE_HPP_
#define WIRE_CORE_GRAMMAR_ENDPOINT_PARSE_HPP_

#include <string>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include <wire/core/endpoint.hpp>
#include <wire/core/grammar/common.hpp>

#include <tip/iri/grammar/iri_parse.hpp>

namespace wire {
namespace core {
namespace grammar {
namespace parse {

struct inet_interface_version_grammar
        : boost::spirit::qi::symbols< char, detail::inet_interface::version_type > {
    inet_interface_version_grammar()
    {
        add
            ("v4",      detail::inet_interface::version_type::v4)
            ("v6",      detail::inet_interface::version_type::v6)
        ;
    }
};

template < typename InputIterator >
struct inet_interface_grammar :
        parser_value_grammar< InputIterator, detail::inet_interface > {
    using value_type = detail::inet_interface;
    inet_interface_grammar() : inet_interface_grammar::base_type(main_rule)
    {
        namespace qi = boost::spirit::qi;
        namespace phx = boost::phoenix;
        using qi::char_;
        using qi::_val;
        using qi::no_case;
        using qi::_1;
        using qi::_2;

        name %= qi::char_("a-zA-Z") >> *qi::char_("a-zA-Z0-9");
        main_rule = name [ phx::bind(&value_type::name, _val) = _1 ]
                >> '['
                >> no_case[ version ][ phx::bind(&value_type::version, _val) = _1 ]
                >> ']';
    }

    parser_value_rule< InputIterator, value_type >      main_rule;
    parser_value_rule< InputIterator, ::std::string >   name;
    inet_interface_version_grammar                      version;
};

struct get_iface_addr_func {
    using result = ::std::string;

    result
    operator()(detail::inet_interface const& iface) const
    {
        return iface.resolve_address();
    }
};

::boost::phoenix::function< get_iface_addr_func > const get_iface_addr
         = get_iface_addr_func{};

struct transport_type_grammar :
        boost::spirit::qi::symbols< char, transport_type > {
    transport_type_grammar()
    {
        add
            ("tcp",        transport_type::tcp)
            ("ssl",        transport_type::ssl)
            ("udp",        transport_type::udp)
            ("socket",    transport_type::socket)
        ;
    }
};

template < typename InputIterator, typename OutType >
struct ip_endpoint_data_grammar :
        parser_value_grammar< InputIterator, OutType > {
    using value_type = OutType;
    ip_endpoint_data_grammar() : ip_endpoint_data_grammar::base_type(main_rule)
    {
        namespace qi = boost::spirit::qi;
        namespace phx = boost::phoenix;
        using qi::digit;
        using qi::eps;
        using qi::char_;
        using qi::_val;
        using qi::_1;
        using qi::_2;

        port = boost::spirit::qi::uint_parser< std::uint16_t, 10, 1 >();
        main_rule = (iface[ phx::bind( &value_type::host, _val ) = get_iface_addr(_1) ] |
                    ihost[ phx::bind( &value_type::host, _val ) = _1 ]) >>
                qi::lit(':')
                >> port[ phx::bind( &value_type::port, _val ) = _1 ];
    }
    parser_value_rule< InputIterator, value_type >                  main_rule;
    inet_interface_grammar< InputIterator >                         iface;
    tip::iri::grammar::parse::ihost_str_grammar< InputIterator >    ihost;
    parser_value_rule< InputIterator, uint16_t >                    port;
};

template < typename InputIterator >
struct socket_endpoint_grammar :
        parser_value_grammar< InputIterator, detail::socket_endpoint_data > {
    using value_type = detail::socket_endpoint_data;
    socket_endpoint_grammar() : socket_endpoint_grammar::base_type(main_rule)
    {
        namespace qi = boost::spirit::qi;
        namespace phx = boost::phoenix;
        using qi::lit;
        using qi::char_;
        using qi::_val;
        using qi::_1;
        path_str %= char_("/") >> isegment_nz >> *(char_("/") >> isegment);
        main_rule = path_str [ phx::bind( &value_type::path, _val ) = _1 ];
    }
    parser_value_rule< InputIterator, value_type>                   main_rule;
    parser_value_rule< InputIterator, std::string >                 path_str;
    tip::iri::grammar::parse::isegment_nz_grammar<InputIterator>    isegment_nz;
    tip::iri::grammar::parse::isegment_grammar<InputIterator>       isegment;
};

template < typename InputIterator >
struct endpoint_grammar :
        parser_value_grammar< InputIterator, endpoint > {
    using value_type = endpoint;
    endpoint_grammar() : endpoint_grammar::base_type(root)
    {
        namespace qi = boost::spirit::qi;
        using qi::lit;
        root =         ( lit("tcp://") >> tcp_endpoint )
                |    ( lit("ssl://") >> ssl_endpoint )
                |    ( lit("udp://") >> udp_endpoint )
                |    ( lit("socket://") >> socket_endpoint )
        ;
    }
    parser_value_rule< InputIterator, value_type >  root;
    ip_endpoint_data_grammar< InputIterator,
        detail::tcp_endpoint_data >                 tcp_endpoint;
    ip_endpoint_data_grammar< InputIterator,
        detail::ssl_endpoint_data >                 ssl_endpoint;
    ip_endpoint_data_grammar< InputIterator,
        detail::udp_endpoint_data >                 udp_endpoint;
    socket_endpoint_grammar< InputIterator >        socket_endpoint;
};

template < typename InputIterator, typename EndpointContainer >
struct endpoints_grammar;

template < typename InputIterator, template < typename ... > class Container, typename ... Rest >
struct endpoints_grammar< InputIterator, Container< endpoint, Rest ... > > :
        parser_value_grammar< InputIterator, Container< endpoint, Rest ... > > {
    using value_type = Container< endpoint, Rest ... >;

    endpoints_grammar() : endpoints_grammar::base_type(root)
    {
        namespace qi = boost::spirit::qi;
        using qi::lit;
        using qi::space;
        using qi::skip;
        using qi::_val;
        using qi::_1;

        additional = *space >> ',' >> *space >> ep[ _val = _1 ];
        root %= ep >> *additional;
    }
    parser_value_rule< InputIterator, value_type >  root;
    parser_value_rule< InputIterator, endpoint >    additional;
    endpoint_grammar< InputIterator >               ep;

};

}  // namespace parse
}  // namespace grammar
}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_GRAMMAR_ENDPOINT_PARSE_HPP_ */
