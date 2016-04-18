/*
 * source_location.hpp
 *
 *  Created on: 18 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_IDL_SOURCE_LOCATION_HPP_
#define WIRE_IDL_SOURCE_LOCATION_HPP_

#include <string>
#include <cstdint>

namespace wire {
namespace idl {

struct location {
    ::std::string file;
    ::std::size_t line;
    ::std::size_t character;
};


}  /* namespace idl */
}  /* namespace wire */


#endif /* WIRE_IDL_SOURCE_LOCATION_HPP_ */
