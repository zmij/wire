/*
 * murmur_hash.hpp
 *
 *  Created on: May 4, 2016
 *      Author: zmij
 */

#ifndef MURMUR_HASH_HPP_
#define MURMUR_HASH_HPP_

#include <cstdint>
#include <string>

namespace wire {
namespace hash {

template < typename T, ::std::size_t bits >
struct murmur_hash_calc;

template < ::std::size_t bits >
struct hash_result {
    using type = void;
    static constexpr ::std::size_t seed = 0;
};

template <>
struct hash_result<16> {
    using type = ::std::uint16_t;
    static constexpr type seed = 0xc005; //49157;

    static type
    bytes(void const* ptr, ::std::size_t length, type seed_val = seed);
};

template <>
struct hash_result<32> {
    using type = ::std::uint32_t;
    static constexpr type seed = 0xc70f6907; // 3339675911

    static type
    bytes(void const* ptr, ::std::size_t length, type seed_val = seed);
};

template <>
struct hash_result<64> {
    using type = ::std::uint64_t;
    static constexpr type seed = 0xc70f6907UL; // 3339675911

    static type
    bytes(void const* ptr, ::std::size_t length, type seed_val = seed);
};

using hash16 = hash_result<16>;
using hash32 = hash_result<32>;
using hash64 = hash_result<64>;

namespace detail {

template < typename _Arg, ::std::size_t bits >
struct hash_base {
    using result_type   = typename hash_result<bits>::type;
    using argument_type = _Arg;
};

}  /* namespace detail */


#define __TRIVIAL_HASH__(_Tp, _Bits)                                    \
template <>                                                             \
struct murmur_hash_calc<_Tp, _Bits> :                                   \
            public detail::hash_base< _Tp, _Bits > {                    \
    result_type                                                         \
    operator()( _Tp __val ) const noexcept                              \
    { return static_cast<result_type>(__val); }                         \
};

// 16-bit
__TRIVIAL_HASH__(bool, 16)
__TRIVIAL_HASH__(char, 16)
__TRIVIAL_HASH__(signed char, 16)
__TRIVIAL_HASH__(unsigned char, 16)
__TRIVIAL_HASH__(wchar_t, 16)
__TRIVIAL_HASH__(char16_t, 16)
__TRIVIAL_HASH__(char32_t, 16)
__TRIVIAL_HASH__(short, 16)
__TRIVIAL_HASH__(int, 16)
__TRIVIAL_HASH__(long, 16)
__TRIVIAL_HASH__(long long, 16)
__TRIVIAL_HASH__(unsigned short, 16)
__TRIVIAL_HASH__(unsigned int, 16)
__TRIVIAL_HASH__(unsigned long, 16)
__TRIVIAL_HASH__(unsigned long long, 16)

// 32-bit
__TRIVIAL_HASH__(bool, 32)
__TRIVIAL_HASH__(char, 32)
__TRIVIAL_HASH__(signed char, 32)
__TRIVIAL_HASH__(unsigned char, 32)
__TRIVIAL_HASH__(wchar_t, 32)
__TRIVIAL_HASH__(char16_t, 32)
__TRIVIAL_HASH__(char32_t, 32)
__TRIVIAL_HASH__(short, 32)
__TRIVIAL_HASH__(int, 32)
__TRIVIAL_HASH__(long, 32)
__TRIVIAL_HASH__(long long, 32)
__TRIVIAL_HASH__(unsigned short, 32)
__TRIVIAL_HASH__(unsigned int, 32)
__TRIVIAL_HASH__(unsigned long, 32)
__TRIVIAL_HASH__(unsigned long long, 32)

// 64-bit
__TRIVIAL_HASH__(bool, 64)
__TRIVIAL_HASH__(char, 64)
__TRIVIAL_HASH__(signed char, 64)
__TRIVIAL_HASH__(unsigned char, 64)
__TRIVIAL_HASH__(wchar_t, 64)
__TRIVIAL_HASH__(char16_t, 64)
__TRIVIAL_HASH__(char32_t, 64)
__TRIVIAL_HASH__(short, 64)
__TRIVIAL_HASH__(int, 64)
__TRIVIAL_HASH__(long, 64)
__TRIVIAL_HASH__(long long, 64)
__TRIVIAL_HASH__(unsigned short, 64)
__TRIVIAL_HASH__(unsigned int, 64)
__TRIVIAL_HASH__(unsigned long, 64)
__TRIVIAL_HASH__(unsigned long long, 64)

#undef __TRIVIAL_HASH__

template < typename _CharT, ::std::size_t bits >
struct murmur_hash_calc < ::std::basic_string<_CharT>, bits > :
        public detail::hash_base< ::std::basic_string<_CharT> const&, bits > {
    using base_type     = detail::hash_base< ::std::basic_string<_CharT> const&, bits >;
    using hash_type     = hash_result<bits>;
    using result_type   = typename base_type::result_type;
    using argument_type = typename base_type::argument_type;

    result_type
    operator()(argument_type arg) const noexcept
    {
        return hash_type::bytes(
            reinterpret_cast<void const*>(arg.data()),
            arg.size() * sizeof(_CharT)
        );
    }
};

template < typename T >
typename hash_result<32>::type
murmur_hash_32(T const& v)
{
    murmur_hash_calc<T, 32> f;
    return f(v);
}

template < typename T >
typename hash_result<64>::type
murmur_hash_64(T const& v)
{
    murmur_hash_calc<T, 64> f;
    return f(v);
}

template < typename T, typename Hash >
auto
combine(T const& v, Hash o)
    -> typename hash_result< sizeof(Hash) * 8 >::type
{
    return murmur_hash_calc<T, sizeof(Hash) * 8 >{}(v) ^ (o << 1);
}

}  /* namespace hash */
}  /* namespace wire */

#endif /* MURMUR_HASH_HPP_ */
