/*
 * cpp_keywords.hpp
 *
 *  Created on: 15 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_WIRE2CPP_KEYWORDS_HPP_
#define WIRE_WIRE2CPP_KEYWORDS_HPP_

#include <string>
#include <set>

namespace wire {
namespace idl {
namespace cpp {

using keywords_type = ::std::set< ::std::string >;

extern keywords_type const keywords;

}  /* namespace cpp */
}  /* namespace idl */
}  /* namespace wire */



#endif /* WIRE_WIRE2CPP_KEYWORDS_HPP_ */
