/*
 * bits.hpp
 *
 *  Created on: Dec 11, 2015
 *      Author: zmij
 */

#ifndef WIRE_DETAIL_BITS_HPP_
#define WIRE_DETAIL_BITS_HPP_

#include <cstdint>
#include <type_traits>

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


#endif /* WIRE_DETAIL_BITS_HPP_ */
