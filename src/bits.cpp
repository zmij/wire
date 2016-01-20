/*
 * bits.cpp
 *
 *  Created on: Dec 11, 2015
 *      Author: zmij
 */

#include <wire/detail/bits.hpp>

namespace tip {
namespace wire {
namespace bits {

inline uint8_t
bit_number(uint8_t byte)
{
	static const uint8_t bits_table[256] = {
#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
		0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
		LT(5), LT(6), LT(6), LT(7), LT(7), LT(7), LT(7),
		LT(8), LT(8), LT(8), LT(8), LT(8), LT(8), LT(8), LT(8)
	};
	return bits_table[byte];
}

uint32_t
significant_bits(uint16_t v)
{
	uint32_t r;
	register unsigned int t;

	r = (t = v >> 8) ? 8 + bit_number(t) : bit_number(v);

	return r;
}

uint32_t
significant_bits(uint32_t v)
{
	uint32_t r;
	register unsigned int t, tt;

	if (tt = v >> 16) {
		r = (t = tt >> 8) ? 24 + bit_number(t) : 16 + bit_number(tt);
	} else {
		r = (t = v >> 8) ? 8 + bit_number(t) : bit_number(v);
	}

	return r;
}

uint32_t
significant_bits(uint64_t v)
{
	uint32_t r;
	register unsigned int t, tt, ttt;

	if (ttt = v >> 32) {
		if (tt = ttt >> 16) {
			r = (t = tt >> 8) ? 56 + bit_number(t) : 48 + bit_number(tt);
		} else {
			r = (t = ttt >> 8) ? 40 + bit_number(t) : 32 + bit_number(ttt);
		}
	} else {
		if (tt = v >> 16) {
			r = (t = tt >> 8) ? 24 + bit_number(t) : 16 + bit_number(tt);
		} else {
			r = (t = v >> 8) ? 8 + bit_number(t) : bit_number(v);
		}
	}
	return r;
}

uint32_t
significant_bits(int16_t v)
{
	return significant_bits(static_cast< uint16_t >( (v << 1) ^ (v >> 15) ));
}

uint32_t
significant_bits(int32_t v)
{
	return significant_bits(static_cast< uint32_t >( (v << 1) ^ (v >> 31) ));
}

uint32_t
significant_bits(int64_t v)
{
	return significant_bits(static_cast< uint64_t >( (v << 1) ^ (v >> 63) ));
}

}  // namespace bits
}  // namespace wire
}  // namespace tip
