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
#include <memory>

namespace wire {
namespace core {

using context_type      = ::std::map< ::std::string, ::std::string >;
using context_ptr       = ::std::shared_ptr< context_type >;
using context_const_ptr = ::std::shared_ptr< context_type const >;

extern const context_type no_context;

}  // namespace core
}  // namespace wire


#endif /* WIRE_CORE_CONTEXT_HPP_ */
