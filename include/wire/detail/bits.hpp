/*
 * bits.hpp
 *
 *  Created on: Dec 11, 2015
 *      Author: zmij
 */

#ifndef TIP_WIRE_DETAIL_BITS_HPP_
#define TIP_WIRE_DETAIL_BITS_HPP_

#include <cstdint>
#include <type_traits>

namespace tip {
namespace wire {
namespace bits {

uint32_t
significant_bits(uint16_t);
uint32_t
significant_bits(uint32_t);
uint32_t
significant_bits(uint64_t);

uint32_t
significant_bits(int16_t);
uint32_t
significant_bits(int32_t);
uint32_t
significant_bits(int64_t);

}  // namespace bits
}  // namespace wire
}  // namespace tip


#endif /* TIP_WIRE_DETAIL_BITS_HPP_ */
