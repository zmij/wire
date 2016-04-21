/*
 * token_types.hpp
 *
 *  Created on: 19 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_IDL_TOKEN_TYPES_HPP_
#define WIRE_IDL_TOKEN_TYPES_HPP_

#include <boost/spirit/include/lex_lexertl.hpp>

namespace wire {
namespace idl {
namespace lexer {

//----------------------------------------------------------------------------
/**
 * Token types
 */
enum token_type {
    token_locn    = ::boost::spirit::lex::min_token_id,
    //@{
    /** @name Keywords */
    token_ns,
    token_struct,
    token_class,
    token_interface,
    token_exception,
    token_const,
    token_using,
    //@}
    //@{
    /** @name Punctuation */
    token_comma,
    token_colon,
    token_scope_resolution,
    token_semicolon,
    token_assign,
    token_asterisk,
    token_brace_open,
    token_brace_close,
    token_block_start,
    token_block_end,
    token_angled_open,
    token_angled_close,
    token_attrib_start,
    token_attrib_end,
    //@}
    token_identifier,
    token_number,
    token_oct_number,
    token_hex_number,
    token_quoted_string,

    token_c_comment,
    token_cpp_comment,

    token_eol,
    token_whitespace,
    token_any
};

template <typename Attribute = ::boost::spirit::lex::unused_type,
        typename Char = char>
using token_def = ::boost::spirit::lex::token_def<Attribute, Char, token_type>;

template <typename Iterator = char const*,
        typename AttributeTypes = ::boost::mpl::vector0<>,
        typename HasState = ::boost::mpl::true_ >
using token = ::boost::spirit::lex::lexertl::token<Iterator, AttributeTypes, HasState, token_type>;

}  /* namespace lexer */
}  /* namespace idl */
}  /* namespace wire */

#endif /* WIRE_IDL_TOKEN_TYPES_HPP_ */
