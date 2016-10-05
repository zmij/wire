/*
 * make_unique.hpp
 *
 *  Created on: Sep 27, 2016
 *      Author: zmij
 */

#ifndef WIRE_UTIL_MAKE_UNIQUE_HPP_
#define WIRE_UTIL_MAKE_UNIQUE_HPP_

#include <memory>

namespace wire {
namespace util {

namespace detail {

template < typename T >
struct _unique_if {
    using _single_object = ::std::unique_ptr<T>;
};

template < typename T >
struct _unique_if<T[]> {
    using _unknown_bound = ::std::unique_ptr<T[]>;
};

template < typename T, size_t N >
struct _unique_if<T[N]> {
    using _known_bound = ::std::unique_ptr<T[N]>;
    static constexpr size_t bound = N;
};

}  /* namespace detail */

template < typename T, typename ... Args >
typename detail::_unique_if<T>::_single_object
make_unique(Args&& ... args)
{
#if __cplusplus == 201103L
    return ::std::unique_ptr<T>{ new T{::std::forward<Args>(args)...} };
#else
#if __cplusplus > 201103L
    return ::std::make_unique<T>(::std::forward<Args>(args)...);
#else
#error This library needs at least a C++11 compliant compiler
#endif
#endif
}

template < typename T >
typename detail::_unique_if<T>::_unknown_bound
make_unique(size_t n)
{
#if __cplusplus == 201103L
    using U = typename ::std::remove_extent<T>::type;
    return ::std::unique_ptr<T>{ new U[n] };
#else
#if __cplusplus > 201103L
    return ::std::make_unique<T>(n);
#else
#error This library needs at least a C++11 compliant compiler
#endif
#endif
}

template < typename T, typename ... Args >
typename detail::_unique_if<T>::_known_bound
make_unique(Args&& ...) = delete;

}  /* namespace util */
}  /* namespace wire */

#endif /* WIRE_UTIL_MAKE_UNIQUE_HPP_ */
