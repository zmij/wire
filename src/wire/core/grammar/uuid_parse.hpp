/*
 * uuid_parse.hpp
 *
 *  Created on: May 5, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_GRAMMAR_UUID_PARSE_HPP_
#define WIRE_CORE_GRAMMAR_UUID_PARSE_HPP_

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include <boost/uuid/uuid.hpp>

#include <wire/core/grammar/common.hpp>

namespace wire {
namespace core {
namespace grammar {
namespace parse {

struct set_uuid_byte_func {
    using result = void;

    void
    operator()(::boost::uuids::uuid& uuid, ::std::size_t byte_no, ::std::uint8_t val) const
    {
        uuid.data[byte_no] = val;
    }

    void
    operator()(::boost::uuids::uuid& uuid, ::std::size_t byte_no, ::std::uint32_t val) const
    {

    }
};

::boost::phoenix::function< set_uuid_byte_func > const set_uuid_byte = set_uuid_byte_func{};

template < typename InputIterator >
struct uuid_grammar : parser_value_grammar< InputIterator, ::boost::uuids::uuid > {
    using value_type = ::boost::uuids::uuid;

    uuid_grammar() : uuid_grammar::base_type(uuid)
    {
        namespace qi = ::boost::spirit::qi;
        namespace phx = ::boost::phoenix;
        using qi::_val;
        using qi::_1;
        using qi::_2;
        using qi::_3;
        using qi::_4;
        using qi::eps;

        byte = qi::uint_parser<::std::uint8_t, 16, 2, 2>();
        uuid = eps[ _val = phx::construct< value_type >() ]
            >> (byte >> byte >> byte >> byte)
                [
                     set_uuid_byte(phx::ref(_val), 0, _1),
                     set_uuid_byte(phx::ref(_val), 1, _2),
                     set_uuid_byte(phx::ref(_val), 2, _3),
                     set_uuid_byte(phx::ref(_val), 3, _4)
                ]
            >> '-'
            >> (byte >> byte)
                [
                 set_uuid_byte(phx::ref(_val), 4, _1),
                 set_uuid_byte(phx::ref(_val), 5, _2)
                ]
            >> '-'
            >> (byte >> byte)
                [
                 set_uuid_byte(phx::ref(_val), 6, _1),
                 set_uuid_byte(phx::ref(_val), 7, _2)
                ]
            >> '-'
            >> (byte >> byte)
                [
                 set_uuid_byte(phx::ref(_val), 8, _1),
                 set_uuid_byte(phx::ref(_val), 9, _2)
                ]
            >> '-'
            >> byte [ set_uuid_byte(phx::ref(_val), 10, _1) ]
            >> byte [ set_uuid_byte(phx::ref(_val), 11, _1) ]
            >> byte [ set_uuid_byte(phx::ref(_val), 12, _1) ]
            >> byte [ set_uuid_byte(phx::ref(_val), 13, _1) ]
            >> byte [ set_uuid_byte(phx::ref(_val), 14, _1) ]
            >> byte [ set_uuid_byte(phx::ref(_val), 15, _1) ]
        ;
    }

    parser_value_rule< InputIterator, value_type >      uuid;
    parser_value_rule< InputIterator, ::std::uint8_t >  byte;
};

}  /* namespace parse */
}  /* namespace grammar */
}  /* namespace core */
}  /* namespace wire */


#endif /* WIRE_CORE_GRAMMAR_UUID_PARSE_HPP_ */
