/*
 * source_advance.hpp
 *
 *  Created on: 25 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_IDL_GRAMMAR_SOURCE_ADVANCE_HPP_
#define WIRE_IDL_GRAMMAR_SOURCE_ADVANCE_HPP_

#include <wire/idl/source_location.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/repository/include/qi_iter_pos.hpp>

namespace wire {
namespace idl {
namespace grammar {

template < typename InputIterator >
struct location_grammar : ::boost::spirit::qi::grammar< InputIterator, source_location() > {
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
        root = eps[phx::bind(&source_location::character, _val) = 0]
           >> lit("#line") >> +space
           >> line[ phx::bind(&source_location::line, _val) = _1 ]
           >> +space >> '"'
           >> file_name[phx::bind(&source_location::file, _val) = _1]
           >> '"' >> "\n";
    }
    ::boost::spirit::qi::rule<InputIterator, source_location()>        root;
    ::boost::spirit::qi::rule<InputIterator, ::std::size_t()>   line;
    ::boost::spirit::qi::rule<InputIterator, ::std::string()>   file_name;
};

//----------------------------------------------------------------------------
//  Phoenix function adaptors
//----------------------------------------------------------------------------
struct source_location_func {
    using result = void;
    template < typename TokenValue >
    void
    operator()(idl::source_location& loc, TokenValue const& tok) const
    {
        namespace qi = ::boost::spirit::qi;
        using Iterator = decltype(tok.begin());
        using grammar_type = location_grammar<Iterator>;

        auto begin = tok.begin();
        auto end = tok.end();

        if (!qi::parse(begin, end, grammar_type{}, loc) || begin != end) {
            throw ::std::runtime_error("Invalid preprocessor location string");
        }
    }
};

::boost::phoenix::function< source_location_func > const update_location
      = source_location_func{};

//----------------------------------------------------------------------------
struct advance_location_func {
    using result = void;

    template < typename TokenValue >
    void
    operator()(idl::source_location& loc, TokenValue const& tok) const
    {
        loc.character += ::std::distance(tok.begin(), tok.end());
    }

    void
    operator()(idl::source_location& loc, ::std::size_t sz) const
    {
        loc.character += sz;
    }
};

::boost::phoenix::function< advance_location_func > const al
      = advance_location_func{};

//----------------------------------------------------------------------------
struct next_line_func {
    using result = void;

    void
    operator()(idl::source_location& loc) const
    {
        loc.character = 0;
        ++loc.line;
    }
};

::boost::phoenix::function< next_line_func > const nl = next_line_func{};

//----------------------------------------------------------------------------
template < typename InputIterator >
struct current_pos {
    using base_iterator = decltype( ::std::declval<InputIterator>()->value().begin() );
    current_pos()
    {
        namespace qi = ::boost::spirit::qi;
        namespace phx = ::boost::phoenix;
        using ::boost::spirit::repository::qi::iter_pos;

        save_start_pos = qi::omit[ iter_pos[
             phx::bind( &current_pos::set_start_pos, this, qi::_1 )] ];
        current = iter_pos[ qi::_val = phx::bind( &current_pos::get_current_pos, this, qi::_1 ) ];
    }
    ::boost::spirit::qi::rule<InputIterator>                    save_start_pos;
    ::boost::spirit::qi::rule<InputIterator, ::std::size_t()>   current;
private:
    void
    set_start_pos(InputIterator const& iterator)
    {
        start_pos_ = iterator->value().begin();
    }

    ::std::size_t
    get_current_pos(InputIterator const& iterator)
    {
        return ::std::distance(start_pos_, iterator->value().begin());
    }

    base_iterator start_pos_;
};

}  /* namespace grammar */
}  /* namespace idl */
}  /* namespace wire */

#endif /* WIRE_IDL_GRAMMAR_SOURCE_ADVANCE_HPP_ */
