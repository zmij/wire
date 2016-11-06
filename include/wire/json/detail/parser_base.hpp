/*
 * parser_fwd.hpp
 *
 *  Created on: 24 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_PARSER_BASE_HPP_
#define WIRE_JSON_DETAIL_PARSER_BASE_HPP_

#include <string>
#include <cstdint>
#include <exception>
#include <memory>
#include <iosfwd>

#include <wire/json/json_io_base.hpp>
#include <wire/json/detail/parser_base_fwd.hpp>

namespace wire {
namespace json {
namespace detail {

enum class parse_result {
    need_more,
    done
};

template < typename CharT, typename Traits >
struct basic_parser_base {
    using this_type         = basic_parser_base<CharT, Traits>;
    using parser_ptr_type   = ::std::shared_ptr<this_type>;
    using string_type       = ::std::basic_string<CharT, Traits>;

    virtual ~basic_parser_base() {}

    ::std::size_t
    stack_size() const
    {
        if (current_parser_)
            return current_parser_->stack_size() + 1;
        return 0;
    }

    virtual parse_result
    string_literal(string_type const& val)
    {
        if (current_parser_) {
            if (current_parser_->string_literal(val) == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        throw ::std::runtime_error{ "Unexpected string literal" };
    }
    virtual parse_result
    integral_literal(::std::int64_t val)
    {
        if (current_parser_) {
            if (current_parser_->integral_literal(val) == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        throw ::std::runtime_error{ "Unexpected integral literal" };
    }
    virtual parse_result
    float_literal(long double val)
    {
        if (current_parser_) {
            if (current_parser_->float_literal(val) == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        throw ::std::runtime_error{ "Unexpected float literal" };
    }
    virtual parse_result
    bool_literal(bool val)
    {
        if (current_parser_) {
            if (current_parser_->bool_literal(val) == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        throw ::std::runtime_error{ "Unexpected bool literal" };
    }
    virtual parse_result
    null_literal()
    {
        if (current_parser_) {
            if (current_parser_->null_literal() == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        throw ::std::runtime_error{ "Unexpected null literal" };
    }

    virtual parse_result
    start_array()
    {
        if (current_parser_) {
            if (current_parser_->start_array() == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        throw ::std::runtime_error{ "Unexpected array start" };
    }
    virtual parse_result
    end_array()
    {
        if (current_parser_) {
            if (current_parser_->end_array() == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        throw ::std::runtime_error{ "Unexpected array end" };
    }
    virtual parse_result
    start_element()
    {
        if (current_parser_) {
            if (current_parser_->start_element() == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        throw ::std::runtime_error{ "Unexpected array element start" };
    }

    virtual parse_result
    start_object()
    {
        if (current_parser_) {
            if (current_parser_->start_object() == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        throw ::std::runtime_error{ "Unexpected object start" };
    }
    virtual parse_result
    end_object()
    {
        if (current_parser_) {
            if (current_parser_->end_object() == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        throw ::std::runtime_error{ "Unexpected object end" };
    }
    virtual parse_result
    start_member(string_type const& name)
    {
        if (current_parser_) {
            if (current_parser_->start_member(name) == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        // FIXME Report member name, for whcar_t also
        throw ::std::runtime_error{ "Unexpected member start" };
    }
protected:
    parser_ptr_type     current_parser_ = nullptr;
};

template < typename CharT, typename Traits >
struct basic_ignore_parser : basic_parser_base<CharT, Traits> {
    using base_type = basic_parser_base<CharT, Traits>;
    using string_type = typename base_type::string_type;
    using ignore_object_parser  = basic_ignore_object_parser<CharT, Traits>;
    using ignore_array_parser   = basic_ignore_array_parser<CharT, Traits>;

    virtual ~basic_ignore_parser() {}

    parse_result
    string_literal(string_type const& val) override
    {
        if (current_parser_) {
            if (current_parser_->string_literal(val) == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        return parse_result::done;
    }
    parse_result
    integral_literal(::std::int64_t val) override
    {
        if (current_parser_) {
            if (current_parser_->integral_literal(val) == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        return parse_result::done;
    }
    parse_result
    float_literal(long double val) override
    {
        if (current_parser_) {
            if (current_parser_->integral_literal(val) == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        return parse_result::done;
    }
    parse_result
    bool_literal(bool val) override
    {
        if (current_parser_) {
            if (current_parser_->integral_literal(val) == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        return parse_result::done;
    }
    parse_result
    null_literal() override
    {
        if (current_parser_) {
            if (current_parser_->null_literal() == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        return parse_result::done;
    }

    parse_result
    start_array() override
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
    end_array() override
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
    start_object() override
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
    end_object() override
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
protected:
    using base_type::current_parser_;
};

template < typename CharT, typename Traits >
struct basic_ignore_array_parser : basic_ignore_parser<CharT, Traits> {
    using base_type = basic_ignore_parser<CharT, Traits>;

    virtual ~basic_ignore_array_parser() {}

    parse_result
    end_array() override
    {
        if (current_parser_) {
            if (current_parser_->end_array() == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        return parse_result::done;
    }
    parse_result
    start_element() override
    {
        if (current_parser_) {
            if (current_parser_->start_element() == parse_result::done)
                current_parser_ = nullptr;
        }
        return parse_result::need_more;
    }
protected:
    using base_type::current_parser_;
};

template< typename CharT, typename Traits >
struct basic_ignore_object_parser : basic_ignore_parser<CharT, Traits> {
    using base_type = basic_ignore_parser<CharT, Traits>;
    using string_type = typename base_type::string_type;

    virtual ~basic_ignore_object_parser() {}

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
        }
        return parse_result::need_more;
    }
protected:
    using base_type::current_parser_;
};

template < typename CharT, typename Traits >
struct basic_delegate_parser : basic_parser_base<CharT, Traits> {
    using base_type = basic_parser_base<CharT, Traits>;
    using string_type = typename base_type::string_type;

    virtual ~basic_delegate_parser() {}

    parse_result
    string_literal(string_type const& val) override
    {
        if (current_parser_)
            return current_parser_->string_literal(val);
        throw ::std::runtime_error{ "Unexpected string literal" };
    }
    parse_result
    integral_literal(::std::int64_t val) override
    {
        if (current_parser_)
            return current_parser_->integral_literal(val);
        throw ::std::runtime_error{ "Unexpected integral literal" };
    }
    parse_result
    float_literal(long double val) override
    {
        if (current_parser_)
            return current_parser_->float_literal(val);
        throw ::std::runtime_error{ "Unexpected float literal" };
    }
    parse_result
    bool_literal(bool val) override
    {
        if (current_parser_)
            return current_parser_->bool_literal(val);
        throw ::std::runtime_error{ "Unexpected bool literal" };
    }
    parse_result
    null_literal() override
    {
        if (current_parser_)
            return current_parser_->null_literal();
        throw ::std::runtime_error{ "Unexpected null literal" };
    }

    parse_result
    start_array() override
    {
        if (current_parser_)
            return current_parser_->start_array();
        throw ::std::runtime_error{ "Unexpected array start" };
    }
    parse_result
    end_array() override
    {
        if (current_parser_)
            return current_parser_->end_array();
        throw ::std::runtime_error{ "Unexpected array end" };
    }
    parse_result
    start_element() override
    {
        if (current_parser_)
            return current_parser_->start_element();
        throw ::std::runtime_error{ "Unexpected array element start" };
    }

    parse_result
    start_object() override
    {
        if (current_parser_)
            return current_parser_->start_object();
        throw ::std::runtime_error{ "Unexpected object start" };
    }
    parse_result
    end_object() override
    {
        if (current_parser_)
            return current_parser_->end_object();
        throw ::std::runtime_error{ "Unexpected object end" };
    }
    parse_result
    start_member(string_type const& name) override
    {
        if (current_parser_)
            return current_parser_->start_member(name);
        throw ::std::runtime_error{ "Unexpected member start" };
    }
protected:
    using base_type::current_parser_;
};

bool
parse(parser_base&, char const* first, ::std::size_t size);
bool
parse(parser_base&, ::std::string const& data);
bool
parse(parser_base&, ::std::istream& is);

bool
parse(wparser_base&, wchar_t const* first, ::std::size_t size);
bool
parse(wparser_base&, ::std::wstring const& data);
bool
parse(wparser_base&, ::std::wistream& is);

extern template struct basic_parser_base<char>;
extern template struct basic_parser_base<wchar_t>;
extern template struct basic_ignore_parser<char>;
extern template struct basic_ignore_parser<wchar_t>;
extern template struct basic_ignore_array_parser<char>;
extern template struct basic_ignore_array_parser<wchar_t>;
extern template struct basic_ignore_object_parser<char>;
extern template struct basic_ignore_object_parser<wchar_t>;
extern template struct basic_delegate_parser<char>;
extern template struct basic_delegate_parser<wchar_t>;

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_DETAIL_PARSER_BASE_HPP_ */
