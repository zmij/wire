/*
 * generate_options.hpp
 *
 *  Created on: May 13, 2016
 *      Author: zmij
 */

#ifndef WIRE_WIRE2CPP_GENERATE_OPTIONS_HPP_
#define WIRE_WIRE2CPP_GENERATE_OPTIONS_HPP_

#include <string>

namespace wire {
namespace idl {
namespace cpp {

struct generate_options {
    ::std::string    header_include_dir;

    ::std::string    header_output_dir;
    ::std::string    source_output_dir;

    bool             dont_use_hashed_id;
    bool             generate_forwards;
};


}  /* namespace cpp */
}  /* namespace idl */
}  /* namespace wire */



#endif /* WIRE_WIRE2CPP_GENERATE_OPTIONS_HPP_ */
