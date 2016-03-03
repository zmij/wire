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

#include <tip/iri/grammar/iri_parse.hpp>

namespace wire {
namespace core {
namespace grammar {
namespace parse {

struct transport_type_grammar :
		boost::spirit::qi::symbols< char, transport_type > {
	transport_type_grammar()
	{
		add
			("tcp",		transport_type::tcp)
			("ssl",		transport_type::ssl)
			("udp",		transport_type::udp)
			("socket",	transport_type::socket)
		;
	}
};

template < typename InputIterator, typename OutType >
struct ip_endpoint_data_grammar :
		boost::spirit::qi::grammar< InputIterator, OutType()> {
	typedef OutType value_type;
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
		main_rule = ihost[ phx::bind( &value_type::host, _val ) = _1 ] >>
				qi::lit(':')
				>> port[ phx::bind( &value_type::port, _val ) = _1 ];
	}
	boost::spirit::qi::rule< InputIterator, value_type()>			main_rule;
	tip::iri::grammar::parse::ihost_str_grammar< InputIterator >	ihost;
	boost::spirit::qi::rule< InputIterator, uint16_t() >			port;
};

template < typename InputIterator >
struct socket_endpoint_grammar :
		boost::spirit::qi::grammar< InputIterator, detail::socket_endpoint_data()> {
	typedef detail::socket_endpoint_data value_type;
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
	boost::spirit::qi::rule< InputIterator, value_type()>			main_rule;
	boost::spirit::qi::rule< InputIterator, std::string() >			path_str;
	tip::iri::grammar::parse::isegment_nz_grammar<InputIterator>	isegment_nz;
	tip::iri::grammar::parse::isegment_grammar<InputIterator>		isegment;
};

template < typename InputIterator >
struct endpoint_grammar :
		boost::spirit::qi::grammar< InputIterator, endpoint()> {
	typedef endpoint value_type;
	endpoint_grammar() : endpoint_grammar::base_type(root)
	{
		namespace qi = boost::spirit::qi;
		using qi::lit;
		root = 		( lit("tcp://") >> tcp_endpoint )
				|	( lit("ssl://") >> ssl_endpoint )
				|	( lit("udp://") >> udp_endpoint )
				|	( lit("socket://") >> socket_endpoint )
		;
	}
	boost::spirit::qi::rule< InputIterator, value_type()>	root;
	ip_endpoint_data_grammar< InputIterator,
		detail::tcp_endpoint_data > 						tcp_endpoint;
	ip_endpoint_data_grammar< InputIterator,
		detail::ssl_endpoint_data >							ssl_endpoint;
	ip_endpoint_data_grammar< InputIterator,
		detail::udp_endpoint_data >							udp_endpoint;
	socket_endpoint_grammar< InputIterator >				socket_endpoint;
};

template < typename InputIterator >
struct endpoints_grammar :
		boost::spirit::qi::grammar< InputIterator, endpoint_list() > {
	typedef endpoint_list value_type;
	endpoints_grammar() : endpoints_grammar::base_type(root)
	{
		namespace qi = boost::spirit::qi;
		using qi::lit;
		using qi::space;
		using qi::skip;
		root %= ep >> *skip(space)[lit(",") >> ep];
	}
	boost::spirit::qi::rule< InputIterator, value_type() >	root;
	endpoint_grammar< InputIterator >						ep;
};

}  // namespace parse
}  // namespace grammar
}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_GRAMMAR_ENDPOINT_PARSE_HPP_ */
