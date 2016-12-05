/*
 * lexer.hpp
 *
 *  Created on: 22 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_JSON_LEXER_HPP_
#define WIRE_JSON_DETAIL_JSON_LEXER_HPP_

#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_algorithm.hpp>
#include <boost/spirit/include/phoenix_core.hpp>

#include <wire/idl/source_location.hpp>
#include <wire/json/json_io_base.hpp>

namespace wire {
namespace json {
namespace lexer {

template < typename Lexer, typename CharT, typename Traits = ::std::char_traits<CharT> >
struct basic_json_tokens : ::boost::spirit::lex::lexer< Lexer > {
    using base          = ::boost::spirit::lex::lexer< Lexer >;
    using json_io       = json_io_base<CharT, Traits>;
    using string_type   = ::std::basic_string<CharT, Traits>;
    using chars         = typename json_io::chars;
    using base::self;

    template < typename T, typename ... U >
    using token_def     = ::boost::spirit::lex::token_def<T, CharT, U... >;
    using empty_token   = ::boost::spirit::lex::token_def<
                                ::boost::spirit::unused_type, CharT>;

    basic_json_tokens()
        : string_literal    {json_io::string_re,            json_io::id_string       },
          empty_string      {json_io::empty_re,             json_io::id_empty_string },
          integral_literal  {json_io::integral_re,          json_io::id_integral     },
          float_literal     {json_io::float_re,             json_io::id_float        },
          true_             {json_io::true_str,             json_io::id_true         },
          false_            {json_io::false_str,            json_io::id_false        },
          null              {json_io::null_str,             json_io::id_null         },
          eol               {json_io::cvt(chars::newline),  json_io::id_newline      },
          ws                {json_io::ws_re,                json_io::id_ws           }
    {
        self = string_literal | empty_string
            | integral_literal | float_literal
            | true_ | false_ | null | eol | ws
            | json_io::cvt(chars::start_object)
            | json_io::cvt(chars::end_object)
            | json_io::cvt(chars::start_array)
            | json_io::cvt(chars::end_array)
            | json_io::cvt(chars::colon)
            | json_io::cvt(chars::comma)
        ;
//        self(json_io::ws_state) = empty_token(json_io::ws_re)
//            | json_io::c_comment_re     // C-style comments
//            | json_io::cpp_comment_re   // C++-style comments
//        ;
    }

    token_def< string_type >    string_literal, empty_string;
    token_def< long >           integral_literal;
    token_def< long double >    float_literal;
    token_def< bool >           true_, false_;
    empty_token                 null, eol, ws;
};

template <typename Lexer>
using json_tokens   = basic_json_tokens<Lexer, char>;
template <typename Lexer>
using wjson_tokens  = basic_json_tokens<Lexer, wchar_t>;

}  /* namespace lexer */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_DETAIL_JSON_LEXER_HPP_ */
