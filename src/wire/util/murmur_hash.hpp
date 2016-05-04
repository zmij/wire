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

using hash_result = ::std::uint64_t;

namespace detail {

template < typename _Arg >
struct hash_base {
    using result_type   = hash_result;
    using argument_type = _Arg;
};

hash_result
murmur_bytes(void const* ptr, ::std::size_t length,
        hash_result seed = static_cast< hash_result >( 0xc70f6907UL ));

}  /* namespace detail */

template < typename T >
struct murmur_hash_calc;

#define __TRIVIAL_HASH__(_Tp)                                       \
template <>                                                         \
struct murmur_hash_calc<_Tp> : public detail::hash_base< _Tp > {    \
    result_type                                                     \
    operator()( _Tp __val ) const noexcept                          \
    { return static_cast<result_type>(__val); }                     \
};

__TRIVIAL_HASH__(bool)
__TRIVIAL_HASH__(char)
__TRIVIAL_HASH__(signed char)
__TRIVIAL_HASH__(unsigned char)
__TRIVIAL_HASH__(wchar_t)
__TRIVIAL_HASH__(char16_t)
__TRIVIAL_HASH__(char32_t)
__TRIVIAL_HASH__(short)
__TRIVIAL_HASH__(int)
__TRIVIAL_HASH__(long)
__TRIVIAL_HASH__(long long)
__TRIVIAL_HASH__(unsigned short)
__TRIVIAL_HASH__(unsigned int)
__TRIVIAL_HASH__(unsigned long)
__TRIVIAL_HASH__(unsigned long long)

#undef __TRIVIAL_HASH__

template < typename _CharT >
struct murmur_hash_calc < ::std::basic_string<_CharT> > :
        public detail::hash_base< ::std::basic_string<_CharT> const& > {
    using base_type     = detail::hash_base< ::std::basic_string<_CharT> const& >;
    using result_type   = typename base_type::result_type;
    using argument_type = typename base_type::argument_type;

    result_type
    operator()(argument_type arg) const noexcept
    {
        return detail::murmur_bytes(
            reinterpret_cast<void const*>(arg.data()),
            arg.size() * sizeof(_CharT)
        );
    }
};

template < typename T >
hash_result
murmur_hash(T const& v)
{
    murmur_hash_calc<T> f;
    return f(v);
}

}  /* namespace hash */
}  /* namespace wire */

#endif /* MURMUR_HASH_HPP_ */
