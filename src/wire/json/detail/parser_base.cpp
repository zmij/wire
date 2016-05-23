/*
 * parser_base.cpp
 *
 *  Created on: 24 мая 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/json/detail/parser_base.hpp>

namespace wire {
namespace json {
namespace detail {

//----------------------------------------------------------------------------
//    parser_base implementation
//----------------------------------------------------------------------------
bool
parser_base::string_literal(::std::string const& val)
{
    if (current_parser_) {
        if (current_parser_->string_literal(val))
            current_parser_ = nullptr;
        return false;
    }
    throw ::std::runtime_error{ "Unexpected string literal" };
}

bool
parser_base::integral_literal(::std::int64_t val)
{
    if (current_parser_) {
        if (current_parser_->integral_literal(val))
            current_parser_ = nullptr;
        return false;
    }
    throw ::std::runtime_error{ "Unexpected integral literal" };
}

bool
parser_base::float_literal(long double val)
{
    if (current_parser_) {
        if (current_parser_->float_literal(val))
            current_parser_ = nullptr;
        return false;
    }
    throw ::std::runtime_error{ "Unexpected float literal" };
}

bool
parser_base::bool_literal(bool val)
{
    if (current_parser_) {
        if (current_parser_->bool_literal(val))
            current_parser_ = nullptr;
        return false;
    }
    throw ::std::runtime_error{ "Unexpected bool literal" };
}

bool
parser_base::null_literal()
{
    if (current_parser_) {
        if (current_parser_->null_literal())
            current_parser_ = nullptr;
        return false;
    }
    throw ::std::runtime_error{ "Unexpected null literal" };
}

bool
parser_base::start_member(::std::string const& name)
{
    if (current_parser_) {
        if (current_parser_->start_member(name))
            current_parser_ = nullptr;
        return false;
    }
    throw ::std::runtime_error{ "Unexpected member start" };
}

bool
parser_base::start_array()
{
    if (current_parser_) {
        if (current_parser_->start_array())
            current_parser_ = nullptr;
        return false;
    }
    throw ::std::runtime_error{ "Unexpected array start" };
}

bool
parser_base::end_array()
{
    if (current_parser_) {
        if (current_parser_->end_array())
            current_parser_ = nullptr;
        return false;
    }
    throw ::std::runtime_error{ "Unexpected array end" };
}

bool
parser_base::start_object()
{
    if (current_parser_) {
        if (current_parser_->start_object())
            current_parser_ = nullptr;
        return false;
    }
    throw ::std::runtime_error{ "Unexpected object start" };
}

bool
parser_base::end_object()
{
    if (current_parser_) {
        if (current_parser_->end_object())
            current_parser_ = nullptr;
        return false;
    }
    throw ::std::runtime_error{ "Unexpected object end" };
}

//----------------------------------------------------------------------------
//    ignore_parser implementation
//----------------------------------------------------------------------------
bool
ignore_parser::string_literal(::std::string const& val)
{
    if (current_parser_) {
        if (current_parser_->string_literal(val))
            current_parser_ = nullptr;
        return false;
    }
    return true;
}

bool
ignore_parser::integral_literal(::std::int64_t val)
{
    if (current_parser_) {
        if (current_parser_->integral_literal(val))
            current_parser_ = nullptr;
        return false;
    }
    return true;
}

bool
ignore_parser::float_literal(long double val)
{
    if (current_parser_) {
        if (current_parser_->integral_literal(val))
            current_parser_ = nullptr;
        return false;
    }
    return true;
}

bool
ignore_parser::bool_literal(bool val)
{
    if (current_parser_) {
        if (current_parser_->integral_literal(val))
            current_parser_ = nullptr;
        return false;
    }
    return true;
}

bool
ignore_parser::null_literal()
{
    if (current_parser_) {
        if (current_parser_->null_literal())
            current_parser_ = nullptr;
        return false;
    }
    return true;
}

bool
ignore_parser::start_array()
{
    if (current_parser_) {
        if (current_parser_->start_array())
            current_parser_ = nullptr;
    } else {
        current_parser_.reset(new ignore_array_parser{});
    }
    return false;
}

bool
ignore_parser::end_array()
{
    if (current_parser_) {
        if (current_parser_->end_array()) {
            current_parser_ = nullptr;
            return true;
        }
        return false;
    }
    throw ::std::runtime_error{ "Unexpected array end" };
}

bool
ignore_parser::start_object()
{
    if (current_parser_) {
        if (current_parser_->start_object())
            current_parser_ = nullptr;
    } else {
        current_parser_.reset(new ignore_object_parser{});
    }
    return false;
}

bool
ignore_parser::end_object()
{
    if (current_parser_) {
        if (current_parser_->end_object()) {
            current_parser_ = nullptr;
            return true;
        }
        return false;
    }
    throw ::std::runtime_error{ "Unexpected object end" };
}

//----------------------------------------------------------------------------
//    ignore_array_parser implementation
//----------------------------------------------------------------------------
bool
ignore_array_parser::end_array()
{
    if (current_parser_) {
        if (current_parser_->end_array())
            current_parser_ = nullptr;
        return false;
    }
    return true;
}

//----------------------------------------------------------------------------
//    ignore_object_parser implementation
//----------------------------------------------------------------------------
bool
ignore_object_parser::end_object()
{
    if (current_parser_) {
        if (current_parser_->end_object())
            current_parser_ = nullptr;
        return false;
    }
    return true;
}

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */
