/*
 * bits_tests.cpp
 *
 *  Created on: Dec 10, 2015
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/encoding/detail/bits.hpp>

TEST(Bits, SignificantBits32)
{
	using namespace wire::bits;

	EXPECT_EQ(0, significant_bits(0u));
	EXPECT_EQ(1, significant_bits(1u));
	EXPECT_EQ(2, significant_bits(2u));
	EXPECT_EQ(2, significant_bits(3u));
	EXPECT_EQ(3, significant_bits(4u));
	EXPECT_EQ(5, significant_bits(16u));
	EXPECT_EQ(11, significant_bits(1024u));
	EXPECT_EQ(11, significant_bits(1040u));

	EXPECT_EQ(29, significant_bits(static_cast<uint32_t>(1 << 28)));

	EXPECT_EQ(1, significant_bits(static_cast<int32_t>(-1)));
	EXPECT_EQ(2, significant_bits(static_cast<int32_t>(1)));

	int32_t v = -5;
	uint32_t e = static_cast<uint32_t>((v << 1) ^ (v >> 31));
	EXPECT_EQ(v, static_cast<int32_t>( (e >> 1) ^ (static_cast<int32_t>(e) << 31 >> 31 ) ));
}

TEST(Bits, SignificantBits64)
{
	using namespace wire::bits;

	EXPECT_EQ(0, significant_bits((uint64_t)0ul));
	EXPECT_EQ(1, significant_bits((uint64_t)1ul));
	EXPECT_EQ(2, significant_bits((uint64_t)2ul));
	EXPECT_EQ(2, significant_bits((uint64_t)3ul));
	EXPECT_EQ(3, significant_bits((uint64_t)4ul));
	EXPECT_EQ(5, significant_bits((uint64_t)16ul));
	EXPECT_EQ(11, significant_bits((uint64_t)1024ul));
	EXPECT_EQ(11, significant_bits((uint64_t)1040ul));

	EXPECT_EQ(49, significant_bits(static_cast<uint64_t>(1ul << 48)));
}
