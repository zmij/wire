/*
 * object_parser.hpp
 *
 *  Created on: 6 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_OBJECT_PARSER_HPP_
#define WIRE_JSON_DETAIL_OBJECT_PARSER_HPP_

#include <wire/json/detail/parser_base.hpp>
#include <unordered_map>

namespace wire {
namespace json {
namespace detail {

template < typename CharT, typename Traits = ::std::char_traits<CharT> >
struct basic_object_parser : basic_parser_base<CharT, Traits> {
    using base_type     = basic_parser_base<CharT, Traits>;
    using string_type   = typename base_type::string_type;
    using ignore_parser = basic_ignore_parser<CharT, Traits>;
    using parser_ptr_type = typename base_type::parser_ptr_type;

    basic_object_parser() {}
    virtual ~basic_object_parser() {}

    parse_result
    end_object() override
    {
        if (current_parser_) {
            if (current_parser_->end_object() == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        return parse_result::done;
    }
    parse_result
    start_member(string_type const& name) override
    {
        if (current_parser_) {
            if (current_parser_->start_member(name) == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        } else {
            auto f = members_.find(name);
            if (f != members_.end()) {
                current_parser_ = f->second;
            } else if (catch_all_parser_) {
                return catch_all_parser_->start_member(name);
            } else {
                // Set current parser to ignore parser
                current_parser_ = ::std::make_shared<ignore_parser>();
            }
        }
        return parse_result::need_more;
    }
    void
    add_member_parser(string_type&& name, parser_ptr_type p) override
    { members_.emplace(::std::move(name), p); }
    void
    add_element_parser(parser_ptr_type p) override
    { catch_all_parser_ = p; }
protected:
    using base_type::current_parser_;
    using member_parsers = ::std::unordered_map<string_type, parser_ptr_type>;
    member_parsers      members_;
    parser_ptr_type     catch_all_parser_;
};

template < typename CharT, typename Traits = ::std::char_traits<CharT> >
struct basic_root_parser : basic_parser_base<CharT, Traits> {
    using base_type     = basic_parser_base<CharT, Traits>;
    using string_type   = typename base_type::string_type;
    using parser_ptr_type = typename base_type::parser_ptr_type;

    virtual ~basic_root_parser() {}

    parse_result
    string_literal(string_type const& val) override
    {
        if (!current_parser_)
            current_parser_ = parser_;
        return current_parser_->string_literal(val);
    }
    parse_result
    integral_literal(::std::int64_t val) override
    {
        if (!current_parser_)
            current_parser_ = parser_;
        return current_parser_->integral_literal(val);
    }
    parse_result
    float_literal(long double val) override
    {
        if (!current_parser_)
            current_parser_ = parser_;
        return current_parser_->float_literal(val);
    }
    parse_result
    bool_literal(bool val) override
    {
        if (!current_parser_)
            current_parser_ = parser_;
        return current_parser_->bool_literal(val);
    }
    parse_result
    null_literal() override
    {
        if (!current_parser_)
            current_parser_ = parser_;
        return current_parser_->null_literal();
    }

    parse_result
    start_array() override
    {
        if (!current_parser_) {
            current_parser_ = parser_;
            return parse_result::need_more;
        }
        return current_parser_->start_array();
    }
    parse_result
    end_array() override
    {
        if (!current_parser_)
            current_parser_ = parser_;
        return current_parser_->end_array();
    }

    parse_result
    start_object() override
    {
        if (!current_parser_) {
            current_parser_ = parser_;
            return parse_result::need_more;
        }
        return current_parser_->start_object();
    }
    parse_result
    end_object() override
    {
        if (!current_parser_)
            current_parser_ = parser_;
        return current_parser_->end_object();
    }

    void
    add_element_parser(parser_ptr_type p) override
    {
        parser_ = p;
    }
protected:
    using base_type::current_parser_;
private:
    parser_ptr_type parser_;
};

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_DETAIL_OBJECT_PARSER_HPP_ */
