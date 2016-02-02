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

namespace detail {

template <typename T>
struct has_call_operator {
private:
    struct _fallback { void operator()(); };
    struct _derived : T, _fallback {};

    template<typename U, U> struct _check;

    template<typename>
    static std::true_type test(...);

    template<typename C>
    static std::false_type test(
    		_check<void (_fallback::*)(), &C::operator()>*);

public:
    static const bool value =
    		std::is_same< decltype(test<_derived>(0)), std::true_type >::value;
};

}  // namespace detail

template < typename T >
struct is_callable : std::conditional<
	std::is_class< T >::value,
	detail::has_call_operator< T >,
	std::is_function<T> >::type {
};

template < typename ... T >
struct function_traits;

template < typename T >
struct not_a_function {
	static const int arity = -1;
};

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
struct call_operator_traits : function_traits< decltype(&T::operator()) > {};

template < typename T >
struct function_traits<T> :
	std::conditional<
		is_callable< T >::value,
		call_operator_traits< T >,
		not_a_function<T> >::type {};

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
