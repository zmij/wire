/*
 * lexer.hpp
 *
 *  Created on: Apr 18, 2016
 *      Author: zmij
 */

#ifndef WIRE_IDL_LEXER_HPP_
#define WIRE_IDL_LEXER_HPP_

#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_algorithm.hpp>
#include <boost/spirit/include/phoenix_core.hpp>

#include <wire/idl/source_location.hpp>
#include <wire/idl/token_types.hpp>

namespace wire {
namespace idl {
namespace lexer {

template < typename InputIterator >
struct location_grammar : ::boost::spirit::qi::grammar< InputIterator, location() > {
    location_grammar() : location_grammar::base_type(root)
    {
        namespace qi = boost::spirit::qi;
        namespace phx = boost::phoenix;
        using qi::lit;
        using qi::space;
        using qi::_val;
        using qi::_1;
        using qi::eps;
        using qi::char_;

        line = ::boost::spirit::qi::uint_parser< ::std::size_t, 10, 1 >();
        file_name %= +(~char_('"'));
        root = eps[phx::bind(&location::character, _val) = 0]
           >> lit("#line") >> +space
           >> line[ phx::bind(&location::line, _val) = _1 ]
           >> +space >> '"'
           >> file_name[phx::bind(&location::file, _val) = _1]
           >> '"' >> "\n";
    }
    ::boost::spirit::qi::rule<InputIterator, location()>        root;
    ::boost::spirit::qi::rule<InputIterator, ::std::size_t()>   line;
    ::boost::spirit::qi::rule<InputIterator, ::std::string()>   file_name;
};

//----------------------------------------------------------------------------
//  Phoenix function adaptors
//----------------------------------------------------------------------------
struct file_location_func {
    using result = void;

    template < typename Iterator >
    void
    operator()(location& loc, Iterator begin, Iterator end) const
    {
        namespace qi = ::boost::spirit::qi;
        using grammar_type = location_grammar<Iterator>;

        if (!qi::parse(begin, end, grammar_type{}, loc) || begin != end) {
            throw ::std::runtime_error("Invalid preprocessor location string");
        }
    }
};

::boost::phoenix::function< file_location_func > const update_location = file_location_func{};

struct distance_func {
    template < typename Iterator >
    auto
    operator()( Iterator const& begin, Iterator const& end ) const
    -> decltype(::std::distance(begin, end))
    {
        return ::std::distance(begin, end);
    }
};

::boost::phoenix::function< distance_func > const distance = distance_func{};

template <typename Attribute = ::boost::spirit::lex::unused_type,
        typename Char = char>
using token_def = ::boost::spirit::lex::token_def<Attribute, Char, token_type>;

template <typename Iterator = char const*,
        typename AttributeTypes = ::boost::mpl::vector0<>,
        typename HasState = ::boost::mpl::true_ >
using token = ::boost::spirit::lex::lexertl::token<Iterator, AttributeTypes, HasState, token_type>;

template < typename Lexer >
struct wire_tokens : ::boost::spirit::lex::lexer< Lexer > {
    using base_type = ::boost::spirit::lex::lexer< Lexer >;
    using base_type::self;

    wire_tokens()
        : current_location{"", 0, 0},
          preproc_directive{"#line[ \t]+\\d+[ \t]+\\\"[^\\\"]+\\\"\n",
                    token_locn},

          ns            {"namespace",               token_ns},
          struct_       {"struct",                  token_struct},
          class_        {"class",                   token_class},
          interface     {"interface",               token_interface},
          exception     {"exception",               token_exception},

          const_        {"const",                   token_const},
          using_        {"using",                   token_using},

          comma         {",",                       token_comma},
          colon         {":",                       token_colon},
          scope         {"::",                      token_scope_resolution},
          semicolon     {";",                       token_semicolon},
          assign        {"=",                       token_assign},
          asterisk      {"\\*",                     token_asterisk},
          brace_open    {"\\(",                     token_brace_open},
          brace_close   {"\\)",                     token_brace_close},
          block_start   {"\\{",                     token_block_start},
          block_end     {"\\}",                     token_block_end},
          angled_open   {"<",                       token_angled_open},
          angled_close  {">",                       token_angled_close},
          attrib_start  {"\\[\\[",                  token_attrib_start},
          attrib_end    {"\\]\\]",                  token_attrib_end},

          identifier    {"[a-zA-Z_][a-zA-Z0-9_]+",  token_identifier},
          number        {"[1-9][0-9]*",             token_number},
          oct_number    {"0[1-7][0-7]*",            token_oct_number},
          hex_number    {"0[xX][0-9a-fA-F]+",       token_hex_number},

          quoted_str    {"\\\"[^\\\"]+\\\"",        token_quoted_string},

          eol           {"\n",                      token_eol},
          whitespace    {"[ \t]",                   token_whitespace},
          any           {".",                       token_any}
    {
        using ::boost::spirit::lex::_start;
        using ::boost::spirit::lex::_end;
        using ::boost::phoenix::ref;

        self =
             preproc_directive [ update_location( ref(current_location), _start, _end ) ]
           | ns             [ ref(current_location.character) += distance(_start, _end) ]

           | struct_        [ ref(current_location.character) += distance(_start, _end) ]
           | class_         [ ref(current_location.character) += distance(_start, _end) ]
           | interface      [ ref(current_location.character) += distance(_start, _end) ]
           | exception      [ ref(current_location.character) += distance(_start, _end) ]

           | const_         [ ref(current_location.character) += distance(_start, _end) ]
           | using_         [ ref(current_location.character) += distance(_start, _end) ]

           | comma          [ ++ref(current_location.character) ]
           | colon          [ ++ref(current_location.character) ]
           | scope          [ ref(current_location.character) += 2 ]
           | semicolon      [ ++ref(current_location.character) ]
           | assign         [ ++ref(current_location.character) ]
           | asterisk       [ ++ref(current_location.character) ]
           | brace_open     [ ++ref(current_location.character) ]
           | brace_close    [ ++ref(current_location.character) ]
           | block_start    [ ++ref(current_location.character) ]
           | block_end      [ ++ref(current_location.character) ]
           | angled_open    [ ++ref(current_location.character) ]
           | angled_close   [ ++ref(current_location.character) ]
           | attrib_start   [ ref(current_location.character) += 2 ]
           | attrib_end     [ ref(current_location.character) += 2 ]

           | identifier     [ ref(current_location.character) += distance(_start, _end) ]
           | number         [ ref(current_location.character) += distance(_start, _end) ]
           | oct_number     [ ref(current_location.character) += distance(_start, _end) ]
           | hex_number     [ ref(current_location.character) += distance(_start, _end) ]
           | quoted_str     [ ref(current_location.character) += distance(_start, _end) ]

           | eol            [ ++ref(current_location.line), ref(current_location.character) = 0 ]
           | whitespace     [ ++ref(current_location.character) ]
           | any            [ ++ref(current_location.character) ]
        ;
    }

    location      current_location;
    token_def<>   preproc_directive;
    //@{
    /** @name Keywords */
    token_def<>   ns;
    token_def<>   struct_;
    token_def<>   class_;
    token_def<>   interface;
    token_def<>   exception;
    token_def<>   const_;
    token_def<>   using_;
    //@}
    //@{
    /** @name Punctuation */
    token_def<>   comma;
    token_def<>   colon;
    token_def<>   scope;
    token_def<>   semicolon;
    token_def<>   assign;
    token_def<>   asterisk;
    token_def<>   brace_open;
    token_def<>   brace_close;
    token_def<>   block_start;
    token_def<>   block_end;
    token_def<>   angled_open;
    token_def<>   angled_close;
    token_def<>   attrib_start;
    token_def<>   attrib_end;
    //@}
    token_def<>   identifier;
    token_def<>   number;
    token_def<>   oct_number;
    token_def<>   hex_number;
    token_def<>   quoted_str;

    token_def<>   eol;
    token_def<>   whitespace;
    token_def<>   any;
};

}  /* namespace lexer */
}  // namespace idl
}  // namespace wire


#endif /* WIRE_IDL_LEXER_HPP_ */
