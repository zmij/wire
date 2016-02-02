/*
 * function_traits.hpp
 *
 *  Created on: Feb 2, 2016
 *      Author: zmij
 */

#ifndef WIRE_UTIL_FUNCTION_TRAITS_HPP_
#define WIRE_UTIL_FUNCTION_TRAITS_HPP_

#include <tuple>
#include <wire/util/meta_helpers.hpp>

namespace wire {
namespace util {

template < typename ... T >
struct function_traits;

template < typename Class, typename Return, typename ... Args >
struct function_traits< Return(Class::*)(Args...) const> {
	enum { arity = sizeof...(Args) };

	typedef Return 					result_type;
	typedef std::tuple< Args ... >	args_tuple_type;
	typedef std::tuple< typename std::decay<Args>::type ... >
									decayed_args_tuple_type;
	template < size_t n>
	struct arg {
		typedef typename std::tuple_element<n, args_tuple_type>::type type;
	};
};

template < typename Class, typename Return, typename Arg >
struct function_traits< Return(Class::*)(Arg) const > {
	enum { arity = 1 };

	typedef Return							result_type;
	typedef Arg								args_tuple_type;
	typedef typename std::decay<Arg>::type	decayed_args_tuple_type;
};

template < typename Class, typename Return >
struct function_traits< Return(Class::*)() const> {
	enum { arity = 0 };
	typedef Return 					result_type;
};

template < typename T >
struct function_traits<T> : function_traits< decltype(&T::operator()) > {};

namespace detail {

template < typename Func, size_t ... Indexes, typename ... T >
typename function_traits<Func>::result_type
invoke(Func func, indexes_tuple< Indexes ... >, std::tuple< T ... >& args)
{
	return func(std::get<Indexes>(args) ...);
}

}  // namespace detail

template < typename Func, typename ... T >
typename function_traits<Func>::result_type
invoke(Func func, std::tuple< T ... >& args)
{
	typedef typename index_builder< sizeof ... (T) >::type	index_type;
	return detail::invoke(func, index_type(), args);
}

}  // namespace util
}  // namespace wire

#endif /* WIRE_UTIL_FUNCTION_TRAITS_HPP_ */
