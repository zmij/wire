/*
 * segment.hpp
 *
 *  Created on: May 3, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_SEGMENT_HPP_
#define WIRE_ENCODING_SEGMENT_HPP_

#include <wire/encoding/wire_io.hpp>
#include <boost/variant.hpp>

namespace wire {
namespace encoding {

struct segment_header {
    using type_id_type = ::boost::variant< ::std::string, hash_value_type >;

    enum flags_type {
        none            = 0x00,
        string_type_id  = 0x01,
        hash_type_id    = 0x02,
        last_segment    = 0x04
    };

    flags_type      flags;
    type_id_type    type_id;
    ::std::size_t   size;
};

}  /* namespace encoding */
}  /* namespace wire */



#endif /* WIRE_ENCODING_SEGMENT_HPP_ */
