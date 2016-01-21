/*
 * static_tests.cpp
 *
 *  Created on: Dec 14, 2015
 *      Author: zmij
 */

#include <type_traits>
#include <wire/encoding/detail/wire_traits.hpp>
#include <wire/encoding/buffers.hpp>

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
static_assert(std::is_same< arg_type_helper< uint8_t >::type, uint8_t >::value,
		"Fundamental type is passed by value");
static_assert(std::is_same< arg_type_helper< std::string >::type, std::string const&>::value,
		"Class type is passed by const reference");
//@}

}  // namespace detail
}  // namespace encoding
}  // namespace wire
