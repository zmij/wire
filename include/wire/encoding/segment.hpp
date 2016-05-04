/*
 * segment.hpp
 *
 *  Created on: May 3, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_SEGMENT_HPP_
#define WIRE_ENCODING_SEGMENT_HPP_

#include <wire/encoding/wire_io.hpp>

namespace wire {
namespace encoding {

struct segment_header {
    enum flags_type {
        none            = 0x00,
        string_type_id  = 0x01,
        last_segment    = 0x04
    };

    flags_type      flags;
    ::std::string   type_id;
    ::std::size_t   size;
};

}  /* namespace encoding */
}  /* namespace wire */



#endif /* WIRE_ENCODING_SEGMENT_HPP_ */
