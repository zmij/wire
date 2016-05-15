/*
 * lexer_qi_test.cpp
 *
 *  Created on: 25 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>
#include "config.hpp"

#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_algorithm.hpp>
#include <boost/spirit/include/phoenix_core.hpp>

#include <wire/idl/preprocess.hpp>
#include <wire/idl/source_location.hpp>
#include <wire/idl/token_types.hpp>
#include <wire/idl/qname.hpp>
#include <wire/idl/grammar/source_advance.hpp>
#include <wire/idl/grammar/idl_file.hpp>

#include <wire/idl/lexer.hpp>

namespace wire {
namespace idl {
namespace grammar {
namespace test {

namespace lex = ::boost::spirit::lex;
namespace qi = ::boost::spirit::qi;


struct idl_file {
    using location_jumps = ::std::map< ::std::size_t, source_location >;
    using base_iterator = char const*;
    using attribs = ::boost::mpl::vector0<>;
    using token_type = lex::lexertl::token<base_iterator, attribs, ::boost::mpl::true_>;
    using lexer_type = lex::lexertl::lexer<token_type>;
    using tokens_type = lexer::wire_tokens< lexer_type >;
    using token_iterator = tokens_type::iterator_type;

    idl_file(::std::string const& contents)
        : stream_begin(contents.data()),
          jumps{ {0, source_location{}} }
    {
    }

    void
    update_location(base_iterator p, source_location const& loc)
    {
        ::std::cerr << "Add location jump stream pos "
                << ::std::distance(stream_begin, p) << " "
                << loc << "\n";
        jumps[ ::std::distance(stream_begin, p) ] = loc;
    }

    source_location
    get_location(base_iterator p) const
    {
        return get_location( ::std::distance(stream_begin, p) );
    }
    source_location
    get_location(::std::size_t pos) const
    {
        auto f = --jumps.upper_bound( pos );
        source_location loc = f->second;
        for (auto c = stream_begin + f->first; c != stream_begin + pos; ++c) {
            if (*c == '\n') {
                loc.character = 0;
                ++loc.line;
            } else {
                ++loc.character;
            }
        }

        return loc;
    }

    void
    add_type_alias(::std::size_t pos, type_alias_decl const& decl)
    {
        ::std::cerr << get_location(pos) << " note: "
                << "Add type alias " << decl.first << " = " << decl.second << "\n";
    }

    void
    forward_declare(::std::size_t pos, fwd_decl const& fwd)
    {
        ::std::cerr << get_location(pos) << " note: "
                << "Forward declare " << fwd.first << " " << fwd.second << "\n";
    }

    void
    declare_enum(::std::size_t pos, enum_decl const& decl)
    {
        ::std::cerr << get_location(pos) << " note: "
                << "Declare enum " << decl.name << "\n";
    }

    void
    add_constant(::std::size_t pos, data_member_decl const& decl)
    {
        ::std::cerr << get_location(pos) << " note: "
                << "Add constant " << decl.type << " " << decl.name << "\n";
    }

    void
    add_data_member(::std::size_t pos, data_member_decl const& decl)
    {
        ::std::cerr << get_location(pos) << " note: "
             << "Add data member " << decl.type << " " << decl.name << "\n";
    }

    void
    add_func_member(::std::size_t pos, function_decl const& decl)
    {
        ::std::cerr << get_location(pos) << " note: "
                 << "Add function member " << decl.return_type << " " << decl.name << "\n";
    }

    void
    end_scope(::std::size_t pos)
    {
        ::std::cerr << get_location(pos) << " note: "<< "End scope\n";
    }

    void
    start_namespace(::std::size_t pos, ::std::string const& name)
    {
        ::std::cerr << get_location(pos) << " note: "
                << "Start namespace " << name << "\n";
    }

    void
    start_structure(::std::size_t pos, ::std::string const& name)
    {
        ::std::cerr << get_location(pos) << " note: "
                << "Start structure " << name << "\n";
    }

    void
    start_interface(::std::size_t pos, ::std::string const& name, ::boost::optional< type_name_list > const& ancestors)
    {
        ::std::cerr << get_location(pos) << " note: "
                << "Start interface " << name;
        if (ancestors.is_initialized()) {
            ::std::cerr << " : ";
            for (auto p = ancestors->begin(); p != ancestors->end(); ++p) {
                if (p != ancestors->begin())
                    ::std::cerr << ", ";
                ::std::cerr << *p;
            }
        }
        ::std::cerr << "\n";
    }

    void
    start_class(::std::size_t pos, ::std::string const& name, ::boost::optional< type_name_list > const& ancestors)
    {
        ::std::cerr << get_location(pos) << " note: "
                << "Start class " << name;
        if (ancestors.is_initialized()) {
            ::std::cerr << " : ";
            for (auto p = ancestors->begin(); p != ancestors->end(); ++p) {
                if (p != ancestors->begin())
                    ::std::cerr << ", ";
                ::std::cerr << *p;
            }
        }
        ::std::cerr << "\n";
    }

    void
    start_exception(::std::size_t pos, ::std::string const& name, ::boost::optional< type_name > const& ancestor)
    {
        ::std::cerr << get_location(pos) << " note: "
                << "Start exception " << name;
        if (ancestor.is_initialized()) {
            ::std::cerr << " : " << *ancestor;
        }
        ::std::cerr << "\n";
    }

    void
    add_annotations(::std::size_t pos, annotation_list const& ann)
    {
        ::std::cerr << get_location(pos) << " note: "
                << "Add annotations (" << ann.size() << ")\n";
    }

    base_iterator  stream_begin;
    location_jumps jumps;
};

TEST(Parser, QiQName)
{
    using base_iterator = char const*;
    using attribs = ::boost::mpl::vector0<>;
    using token_type = lex::lexertl::token<base_iterator, attribs, ::boost::mpl::true_>;
    using lexer_type = lex::lexertl::lexer<token_type>;
    using tokens_type = lexer::wire_tokens< lexer_type >;
    using token_iterator = tokens_type::iterator_type;
    using grammar_type = qname_grammar< token_iterator, tokens_type::lexer_def >;

    source_location sl;

    ::std::string test_str = "test::some_type";
    auto sb = test_str.data();
    auto se = sb + test_str.size();

    tokens_type tokens;
    token_iterator iter = tokens.begin(sb, se);
    token_iterator end = tokens.end();

    grammar_type grammar(tokens);
    ast::qname qn;
    bool r = qi::phrase_parse(iter, end, grammar, qi::in_state("WS")[tokens.self], qn);
    //bool r = qi::parse(iter, end, grammar, qn);

    EXPECT_TRUE(r);
    EXPECT_EQ(iter, end);
    EXPECT_FALSE(qn.fully);
    EXPECT_EQ(2, qn.size());

    ::std::cerr << "Parsed name " << qn << "\n";
}

TEST(Parser, QiTypeName)
{
    using base_iterator = char const*;
    using attribs = ::boost::mpl::vector0<>;
    using token_type = lex::lexertl::token<base_iterator, attribs, ::boost::mpl::true_>;
    using lexer_type = lex::lexertl::lexer<token_type>;
    using tokens_type = lexer::wire_tokens< lexer_type >;
    using token_iterator = tokens_type::iterator_type;
    using grammar_type = type_name_grammar< token_iterator, tokens_type::lexer_def >;

    source_location sl;

    ::std::string test_str = "sequence< array< float, 3 > >";
    auto sb = test_str.data();
    auto se = sb + test_str.size();

    tokens_type tokens;
    token_iterator iter = tokens.begin(sb, se);
    token_iterator end = tokens.end();

    grammar_type grammar(tokens);
    type_name name;
    bool r = qi::phrase_parse(iter, end, grammar, qi::in_state("WS")[tokens.self], name);

    EXPECT_TRUE(r);
    EXPECT_EQ(iter, end);
    EXPECT_FALSE(name.name.fully);
    EXPECT_EQ(1, name.name.size());
    EXPECT_FALSE(name.params.empty());

    if (iter != end) {
        std::cerr << "Errored at state " << ::std::distance(test_str.data(), sb) << "\n";
        for (; sb != se; ++sb) {
            ::std::cerr.put(*sb);
        }
        ::std::cerr << "\n";
    }

    ::std::cerr << "Parsed name " << name << "\n";
}

TEST(Parser, QiNamespace)
{
    using base_iterator = char const*;
    using attribs = ::boost::mpl::vector0<>;
    using token_type = lex::lexertl::token<base_iterator, attribs, ::boost::mpl::true_>;
    using lexer_type = lex::lexertl::lexer<token_type>;
    using tokens_type = lexer::wire_tokens< lexer_type >;
    using token_iterator = tokens_type::iterator_type;
    using grammar_type = idl_file_grammar< token_iterator, tokens_type::lexer_def >;

    const std::string file_name = ::wire::test::DATA_SRC_ROOT + "/wire/interface.wire";

    preprocessor pp { file_name, {{ ::wire::test::IDL_ROOT }, false } };

    std::string input_str = pp.to_string();

    idl_file file{input_str};

    auto sb     = input_str.data();
    auto se     = sb + input_str.size();

    tokens_type tokens;
    token_iterator iter = tokens.begin(sb, se);
    token_iterator end = tokens.end();

    grammar_type grammar(tokens, file);
    bool r = qi::phrase_parse(iter, end, grammar, qi::in_state("WS")[tokens.self]);

    EXPECT_TRUE(r);
    EXPECT_EQ(iter, end);

    if (iter != end) {
        auto loc = file.get_location(sb);
        std::cerr << loc << "\n";
        std::cerr << "Errored at state " << ::std::distance(input_str.data(), sb) << "\n";
        for (; sb != se; ++sb) {
            ::std::cerr.put(*sb);
        }
        ::std::cerr << "\n";
    }
}

}  /* namespace test */
}  /* namespace grammar */
}  /* namespace idl */
}  /* namespace wire */

