/*
 * enum_range.hpp
 *
 *  Created on: Apr 28, 2017
 *      Author: zmij
 */

#ifndef WIRE_UTIL_ENUM_RANGE_HPP_
#define WIRE_UTIL_ENUM_RANGE_HPP_

#include <type_traits>

namespace wire {
namespace util {

namespace detail {
template < typename Enum, Enum ... Enumerators >
struct enumerators {
    static_assert( ::std::is_enum<Enum>::value,
            "Cannot instantiate enumerators for a non-enum" );

    using value_type    = Enum;
    using iterator      = Enum const*;
    using size          = ::std::integral_constant<
                                ::std::size_t, sizeof ... (Enumerators)>;

    static constexpr const value_type   values[] { Enumerators... };
    static constexpr const iterator     begin   = values;
    static constexpr const iterator     end     = values + size::value;

    static constexpr iterator
    position(value_type v)
    {
        iterator left   = begin;
        iterator right  = end;

        while (left != right) {
            iterator mid = left + (right - left) / 2;
            if (v <= *mid) {
                right = mid;
            } else {
                left = mid + 1;
            }
        }
        if (v == *right)
            return right;
        return end;
    }

    static constexpr bool
    valid(value_type v)
    {
        return position(v) != end;
    }
};

template < typename Enum, Enum ... Enumerators >
constexpr const Enum enumerators<Enum, Enumerators...>::values[];

template < typename Enum >
struct enumerators<Enum> {
    static_assert(::std::is_same<Enum, Enum>::value, "Enumeration traits were not defined");
};

struct enum_begin {
    constexpr enum_begin() {}
};
struct enum_end {
    constexpr enum_end() {}
};

} /* namespace detail */

constexpr const detail::enum_begin  enum_begin;
constexpr const detail::enum_end    enum_end;

template < typename Enum >
struct enum_traits {
    using enumerators = detail::enumerators<Enum>;
};

template < typename Enum >
struct enum_value_range {
    using traits_type   = enum_traits<Enum>;
    using enumerators   = typename traits_type::enumerators;
    using value_type    = typename enumerators::value_type;
    using iterator      = typename enumerators::iterator;

    constexpr
    enum_value_range()
        : enum_value_range{ enumerators::begin, enumerators::end } {}
    constexpr
    enum_value_range(value_type first, value_type last)
        : enum_value_range{
            enumerators::position(first), enumerators::position(last) } {}

    constexpr
    enum_value_range(value_type first, detail::enum_end = enum_end)
        : enum_value_range{
            enumerators::position(first), enumerators::end } {}

    constexpr
    enum_value_range(detail::enum_begin, value_type last)
        : enum_value_range{
            enumerators::begin, enumerators::position(last) } {}

    constexpr ::std::size_t
    size() const
    { return end_ - begin_; }
    constexpr bool
    empty() const
    { return begin_ == end_; }

    constexpr iterator
    begin() const
    { return begin_; }
    constexpr iterator
    end() const
    { return end_; }

    constexpr value_type
    front() const
    { return *begin_; }
    constexpr value_type
    back() const
    { return *(end_ - 1); }
private:
    constexpr enum_value_range(iterator first, iterator last)
        : begin_{first}, end_{last} {}
private:
    iterator    begin_;
    iterator    end_;
};

/**
 * Enumeration for-range loop helper
 * @return
 */
template < typename Enum >
constexpr enum_value_range<Enum>
enum_range()
{ return {}; }

/**
 * Enumeration for-range loop helper
 * @return
 */
template < typename Enum >
constexpr enum_value_range<Enum>
enum_range(Enum first, Enum last)
{ return {first, last}; }

/**
 * Enumeration for-range loop helper
 * @return
 */
template < typename Enum >
constexpr enum_value_range< Enum >
enum_range(Enum first, detail::enum_end e = enum_end)
{ return {first, e}; }

/**
 * Enumeration for-range loop helper
 * @return
 */
template < typename Enum >
constexpr enum_value_range< Enum >
enum_range(detail::enum_begin b, Enum last)
{ return {b, last}; }

/**
 * Enumeration size function
 * @return
 */
template < typename Enum >
constexpr ::std::size_t
enum_size()
{ return enum_value_range<Enum>{}.size(); }

/**
 * Check enumerator value is defined in enumeration
 * @param v
 * @return
 */
template < typename Enum >
constexpr bool
enumerator_valid(Enum v)
{ return enum_value_range<Enum>::enumerators::valid(v); }

} /* namespace util */
} /* namespace wire */

#endif /* WIRE_UTIL_ENUM_RANGE_HPP_ */
