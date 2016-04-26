/*
 * parser.cpp
 *
 *  Created on: Apr 20, 2016
 *      Author: zmij
 */

#include <wire/idl/parser.hpp>
#include <iostream>
#include <iomanip>
#include <functional>
#include <boost/optional.hpp>

namespace wire {
namespace idl {
namespace parser {

parser_state::parser_state(::std::string const& contents)
    : stream_begin(contents.data()),
      loc_jumps{ {0, source_location{}} }
{
}

}  // namespace parser
}  // namespace idl
}  // namespace wire
