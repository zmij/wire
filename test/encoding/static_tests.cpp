/*
 * static_tests.cpp
 *
 *  Created on: Dec 14, 2015
 *      Author: zmij
 */

#include <type_traits>
#include <wire/encoding/detail/wire_traits.hpp>
#include <wire/encoding/buffers.hpp>

#include <wire/util/function_traits.hpp>

#include <wire/errors/user_exception.hpp>
#include <wire/core/object.hpp>
#include <wire/core/proxy.hpp>

namespace wire {
namespace encoding {
namespace detail {

//@{
/** @name Wire types */
//@{
/** @name Fixed bytes wire types */
static_assert( wire_type< bool >::value == SCALAR_FIXED,
        "Boolean wire type" );
static_assert( wire_type< char >::value == SCALAR_FIXED,
        "Character wire type" );
static_assert( wire_type< unsigned char >::value == SCALAR_FIXED,
        "Unsigned character wire type" );
static_assert( wire_type< int8_t >::value == SCALAR_FIXED,
        "Int8 wire type" );
static_assert( wire_type< uint8_t >::value == SCALAR_FIXED,
        "UInt8 wire type" );

static_assert( wire_type< float >::value == SCALAR_FIXED,
        "Float wire type");
static_assert( wire_type< double >::value == SCALAR_FIXED,
        "Double wire type");

static_assert( wire_type< int32_fixed_t >::value == SCALAR_FIXED,
        "Fixed int32 wire type");
static_assert( wire_type< uint32_fixed_t >::value == SCALAR_FIXED,
        "Fixed uint32 wire type");
static_assert( wire_type< int64_fixed_t >::value == SCALAR_FIXED,
        "Fixed int32 wire type");
static_assert( wire_type< uint64_fixed_t >::value == SCALAR_FIXED,
        "Fixed int32 wire type");
//@}

//@{
/** @name Varint wire types */
static_assert( wire_type< int16_t >::value == SCALAR_VARINT,
        "int16 wire type");
static_assert( wire_type< int32_t >::value == SCALAR_VARINT,
        "int32 wire type");
static_assert( wire_type< int64_t >::value == SCALAR_VARINT,
        "int64 wire type");
static_assert( wire_type< uint16_t >::value == SCALAR_VARINT,
        "uint16 wire type");
static_assert( wire_type< uint32_t >::value == SCALAR_VARINT,
        "uint32 wire type");
static_assert( wire_type< uint64_t >::value == SCALAR_VARINT,
        "uint64 wire type");
//@}
//@{
static_assert( wire_type< std::string >::value == SCALAR_WITH_SIZE,
        "std::string wire type");
//@}
//@{
static_assert( wire_type< errors::user_exception >::value == EXCEPTION,
        "user_exception wire type");
static_assert( wire_type< errors::user_exception_ptr >::value == EXCEPTION,
        "user_exception wire type");
static_assert( wire_type< core::object >::value == CLASS,
        "object wire type");
static_assert( wire_type< core::object_ptr >::value == CLASS,
        "object wire type");
static_assert( wire_type< core::object_proxy >::value == PROXY,
        "proxy wire type");
static_assert( wire_type< core::object_prx >::value == PROXY,
        "proxy wire type");
//@}
//@}

//@{
/** @name Mask calculation */
static_assert(varint_mask< uint8_t >::value == static_cast< uint8_t >(0x80),
        "Mask for 8 bits");
static_assert(varint_mask< uint16_t >::value == static_cast< uint16_t >(0xff80),
        "Mask for 16 bits");
static_assert(varint_mask< uint32_t >::value == static_cast< uint32_t >(0xffffff80),
        "Mask for 32 bits");
static_assert(varint_mask< uint64_t >::value == static_cast< uint64_t >(0xffffffffffffff80),
        "Mask for 64 bits");
//@}

//@{
/** @name Argument types calculation */
static_assert(std::is_same< arg_type_helper< uint8_t >::in_type, uint8_t >::value,
        "Fundamental type is passed by value for writing");
static_assert(std::is_same< arg_type_helper< uint8_t& >::in_type, uint8_t >::value,
        "Fundamental type is passed by value for writing");
static_assert(std::is_same< arg_type_helper< uint8_t >::out_type, uint8_t& >::value,
        "Fundamental type is passed by reference for reading");

static_assert(std::is_same< arg_type_helper< std::string >::in_type, std::string const&>::value,
        "Class type is passed by const reference for writing");
static_assert(std::is_same< arg_type_helper< std::string >::out_type, std::string&>::value,
        "Class type is passed by reference for reading");
//@}

void
test_lambda_traits()
{
    auto lambda = [](int i, std::string const&){ return uint64_t(i * 42); };
    typedef decltype(lambda) lambda_type;
    typedef util::function_traits< lambda_type > lambda_traits;

    typedef decltype(test_lambda_traits) func_type;
    static_assert(!std::is_class<func_type>::value, "Function is not class");
    static_assert(std::is_function<func_type>::value, "Function is function");
    static_assert(util::is_callable<func_type>::value, "Function is callable");

    static_assert(util::detail::has_call_operator<lambda_type>::value,
            "Lambda has call operator");
    static_assert(std::is_class<lambda_type>::value, "Lambda is class");
    static_assert(util::is_callable<lambda_type>::value, "Lambda is callable");

    static_assert(std::is_same< uint64_t, lambda_traits::result_type >::value,
        "Correct deduced return type");
    static_assert(std::is_same< std::tuple< int, std::string const& >,
            lambda_traits::args_tuple_type >::value,
        "Correct deduced args type");
    static_assert(std::is_same< int, lambda_traits::arg<0>::type>::value,
        "Correct deduced first arg type");
    static_assert(std::is_same< std::string const&, lambda_traits::arg<1>::type>::value,
        "Correct deduced second arg type");
}

typedef std::function< void(std::string const&, bool, int32_t) > test_function_type;
typedef util::function_traits<test_function_type> test_function_traits;

static_assert(util::is_callable<test_function_type>::value, "Functor is callable");
static_assert(std::is_same< void, test_function_traits::result_type >::value,
        "Correct deduced return type");
static_assert(std::is_same<
        std::tuple<std::string const&, bool, int32_t>,
        test_function_traits::args_tuple_type >::value,
        "Correct deduced args type");
static_assert(std::is_same< std::string const&, test_function_traits::arg<0>::type>::value,
    "Correct deduced second arg type");

static_assert(!util::detail::has_call_operator<std::string>::value,
        "String doesn't have a call operator");
static_assert(!util::is_callable<std::string>::value,
        "String is not callable");
static_assert(!util::detail::has_call_operator<outgoing>::value,
        "Outgoing buffer doesn't have a call operator");
static_assert(!util::is_callable<outgoing>::value,
        "Outgoing buffer is not callable");


}  // namespace detail
}  // namespace encoding
}  // namespace wire
