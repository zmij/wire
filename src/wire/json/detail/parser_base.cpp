/*
 * parser_base.cpp
 *
 *  Created on: 24 мая 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/json/detail/parser_base.hpp>
#include <wire/json/detail/parser_traits.hpp>

namespace wire {
namespace json {
namespace detail {

//----------------------------------------------------------------------------
//    parser_base implementation
//----------------------------------------------------------------------------
::std::size_t
parser_base::stack_size() const
{
    if (current_parser_)
        return current_parser_->stack_size() + 1;
    return 0;
}
parse_result
parser_base::string_literal(::std::string const& val)
{
    if (current_parser_) {
        if (current_parser_->string_literal(val) == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    throw ::std::runtime_error{ "Unexpected string literal" };
}

parse_result
parser_base::integral_literal(::std::int64_t val)
{
    if (current_parser_) {
        if (current_parser_->integral_literal(val) == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    throw ::std::runtime_error{ "Unexpected integral literal" };
}

parse_result
parser_base::float_literal(long double val)
{
    if (current_parser_) {
        if (current_parser_->float_literal(val) == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    throw ::std::runtime_error{ "Unexpected float literal" };
}

parse_result
parser_base::bool_literal(bool val)
{
    if (current_parser_) {
        if (current_parser_->bool_literal(val) == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    throw ::std::runtime_error{ "Unexpected bool literal" };
}

parse_result
parser_base::null_literal()
{
    if (current_parser_) {
        if (current_parser_->null_literal() == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    throw ::std::runtime_error{ "Unexpected null literal" };
}

parse_result
parser_base::start_array()
{
    if (current_parser_) {
        if (current_parser_->start_array() == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    throw ::std::runtime_error{ "Unexpected array start" };
}

parse_result
parser_base::end_array()
{
    if (current_parser_) {
        if (current_parser_->end_array() == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    throw ::std::runtime_error{ "Unexpected array end" };
}

parse_result
parser_base::start_element()
{
    if (current_parser_) {
        if (current_parser_->start_element() == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    throw ::std::runtime_error{ "Unexpected array element start" };
}

parse_result
parser_base::start_object()
{
    if (current_parser_) {
        if (current_parser_->start_object() == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    throw ::std::runtime_error{ "Unexpected object start" };
}

parse_result
parser_base::end_object()
{
    if (current_parser_) {
        if (current_parser_->end_object() == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    throw ::std::runtime_error{ "Unexpected object end" };
}

parse_result
parser_base::start_member(::std::string const& name)
{
    if (current_parser_) {
        if (current_parser_->start_member(name) == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    throw ::std::runtime_error{ "Unexpected member " + name + " start" };
}


//----------------------------------------------------------------------------
//    ignore_parser implementation
//----------------------------------------------------------------------------
parse_result
ignore_parser::string_literal(::std::string const& val)
{
    if (current_parser_) {
        if (current_parser_->string_literal(val) == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    return parse_result::done;
}

parse_result
ignore_parser::integral_literal(::std::int64_t val)
{
    if (current_parser_) {
        if (current_parser_->integral_literal(val) == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    return parse_result::done;
}

parse_result
ignore_parser::float_literal(long double val)
{
    if (current_parser_) {
        if (current_parser_->integral_literal(val) == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    return parse_result::done;
}

parse_result
ignore_parser::bool_literal(bool val)
{
    if (current_parser_) {
        if (current_parser_->integral_literal(val) == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    return parse_result::done;
}

parse_result
ignore_parser::null_literal()
{
    if (current_parser_) {
        if (current_parser_->null_literal() == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    return parse_result::done;
}

parse_result
ignore_parser::start_array()
{
    if (current_parser_) {
        if (current_parser_->start_array() == parse_result::done)
            current_parser_ = nullptr;
    } else {
        current_parser_.reset(new ignore_array_parser{});
    }
    return parse_result::need_more;
}

parse_result
ignore_parser::end_array()
{
    if (current_parser_) {
        if (current_parser_->end_array() == parse_result::done) {
            current_parser_ = nullptr;
            return parse_result::done;
        }
        return parse_result::need_more;
    }
    throw ::std::runtime_error{ "Unexpected array end" };
}

parse_result
ignore_parser::start_object()
{
    if (current_parser_) {
        if (current_parser_->start_object() == parse_result::done)
            current_parser_ = nullptr;
    } else {
        current_parser_.reset(new ignore_object_parser{});
    }
    return parse_result::need_more;
}

parse_result
ignore_parser::end_object()
{
    if (current_parser_) {
        if (current_parser_->end_object() == parse_result::done) {
            current_parser_ = nullptr;
            return parse_result::done;
        }
        return parse_result::need_more;
    }
    throw ::std::runtime_error{ "Unexpected object end" };
}

//----------------------------------------------------------------------------
//    ignore_array_parser implementation
//----------------------------------------------------------------------------
parse_result
ignore_array_parser::end_array()
{
    if (current_parser_) {
        if (current_parser_->end_array() == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    return parse_result::done;
}

parse_result
ignore_array_parser::start_element()
{
    if (current_parser_) {
        if (current_parser_->start_element() == parse_result::done)
            current_parser_ = nullptr;
    }
    return parse_result::need_more;
}

//----------------------------------------------------------------------------
//    ignore_object_parser implementation
//----------------------------------------------------------------------------
parse_result
ignore_object_parser::end_object()
{
    if (current_parser_) {
        if (current_parser_->end_object() == parse_result::done)
            current_parser_ = nullptr;
        return parse_result::need_more;
    }
    return parse_result::done;
}

parse_result
ignore_object_parser::start_member(::std::string const& name)
{
    if (current_parser_) {
        if (current_parser_->start_member(name) == parse_result::done)
            current_parser_ = nullptr;
    }
    return parse_result::need_more;
}

//----------------------------------------------------------------------------
//    delegate_parser implementation
//----------------------------------------------------------------------------
parse_result
delegate_parser::string_literal(::std::string const& val)
{
    if (current_parser_)
        return current_parser_->string_literal(val);
    throw ::std::runtime_error{ "Unexpected string literal" };
}

parse_result
delegate_parser::integral_literal(::std::int64_t val)
{
    if (current_parser_)
        return current_parser_->integral_literal(val);
    throw ::std::runtime_error{ "Unexpected integral literal" };
}

parse_result
delegate_parser::float_literal(long double val)
{
    if (current_parser_)
        return current_parser_->float_literal(val);
    throw ::std::runtime_error{ "Unexpected float literal" };
}

parse_result
delegate_parser::bool_literal(bool val)
{
    if (current_parser_)
        return current_parser_->bool_literal(val);
    throw ::std::runtime_error{ "Unexpected bool literal" };
}

parse_result
delegate_parser::null_literal()
{
    if (current_parser_)
        return current_parser_->null_literal();
    throw ::std::runtime_error{ "Unexpected null literal" };
}

parse_result
delegate_parser::start_array()
{
    if (current_parser_)
        return current_parser_->start_array();
    throw ::std::runtime_error{ "Unexpected array start" };
}

parse_result
delegate_parser::end_array()
{
    if (current_parser_)
        return current_parser_->end_array();
    throw ::std::runtime_error{ "Unexpected array end" };
}

parse_result
delegate_parser::start_element()
{
    if (current_parser_)
        return current_parser_->start_element();
    throw ::std::runtime_error{ "Unexpected array element start" };
}

parse_result
delegate_parser::start_object()
{
    if (current_parser_)
        return current_parser_->start_object();
    throw ::std::runtime_error{ "Unexpected object start" };
}

parse_result
delegate_parser::end_object()
{
    if (current_parser_)
        return current_parser_->end_object();
    throw ::std::runtime_error{ "Unexpected object end" };
}

parse_result
delegate_parser::start_member(::std::string const& name)
{
    if (current_parser_)
        return current_parser_->start_member(name);
    throw ::std::runtime_error{ "Unexpected member start" };
}

//----------------------------------------------------------------------------
bool
parse(parser_base& p, char const* first, ::std::size_t size)
{
    namespace qi = ::boost::spirit::qi;
    using parser_traits = detail::parser_traits< char const* >;

    parser_traits::tokenizer_type tokens;
    parser_traits::token_iterator iter = tokens.begin(first, first + size);
    parser_traits::token_iterator end = tokens.end();

    parser_traits::grammar_type grammar(tokens, p);
    return qi::phrase_parse(iter, end, grammar, qi::in_state("WS")[tokens.self]);
}

bool
parse(parser_base& p, ::std::string const& str)
{
    return parse(p, str.data(), str.size());
}

bool
parse(parser_base& p, ::std::istream& is)
{
    namespace qi = ::boost::spirit::qi;
    using istream_iterator = ::boost::spirit::istream_iterator;
    using parser_traits = detail::parser_traits< istream_iterator >;

    istream_iterator sb{is};
    istream_iterator se;

    parser_traits::tokenizer_type tokens;
    parser_traits::token_iterator iter = tokens.begin(sb, se);
    parser_traits::token_iterator end = tokens.end();

    parser_traits::grammar_type grammar(tokens, p);
    return qi::phrase_parse(iter, end, grammar, qi::in_state("WS")[tokens.self]);
}

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */
