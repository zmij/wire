/*
 * idl_file.hpp
 *
 *  Created on: 25 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_IDL_GRAMMAR_IDL_FILE_HPP_
#define WIRE_IDL_GRAMMAR_IDL_FILE_HPP_

#include <wire/idl/grammar/type_name.hpp>
#include <wire/idl/grammar/scope_member.hpp>
#include <wire/idl/grammar/block.hpp>

namespace wire {
namespace idl {
namespace grammar {

template < typename InputIterator, typename Lexer >
struct idl_file_grammar : parser_grammar< InputIterator, Lexer > {
    using main_rule_type = parser_rule< InputIterator, Lexer >;
    using block_rule = block_grammar< InputIterator, Lexer >;

    template < typename TokenDef, typename ParserState >
    idl_file_grammar(TokenDef const& tok, ParserState& state)
        : idl_file_grammar::base_type{ file },
          block{tok, state}
    {
        using update_parser_state = parser_state_update< ParserState >;
        namespace qi = ::boost::spirit::qi;
        using qi::_1;

        update_parser_state up{ state };

        file = *(tok.source_advance[ up.location(_1) ]
            | block);
    }

    main_rule_type file;
    block_rule block;
};

}  /* namespace grammar */
}  /* namespace idl */
}  /* namespace wire */

#endif /* WIRE_IDL_GRAMMAR_IDL_FILE_HPP_ */
