/*
 * context.hpp
 *
 *  Created on: Jan 29, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_CONTEXT_HPP_
#define WIRE_CORE_CONTEXT_HPP_

#include <string>
#include <map>

namespace wire {
namespace core {

typedef ::std::map< ::std::string, ::std::string > context_type;

extern const context_type no_context;

}  // namespace core
}  // namespace wire


#endif /* WIRE_CORE_CONTEXT_HPP_ */
