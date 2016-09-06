/*
 * demangle.hpp
 *
 *  Created on: Sep 6, 2016
 *      Author: zmij
 */

#ifndef WIRE_UTIL_DEMANGLE_HPP_
#define WIRE_UTIL_DEMANGLE_HPP_

#include <cxxabi.h>
#include <string>

namespace wire {
namespace util {

/**
 * Usage:
 * @code
 * ::std::cout << demangle< ::std::iostream >() << "\n"
 * @endcode
 * @return Demangled type name
 */
template < typename T >
::std::string
demangle()
{
    int status {0};
    char* ret = abi::__cxa_demangle( typeid(T).name(), nullptr, nullptr, &status );
    ::std::string res{ret};
    if(ret) free(ret);
    return res;
}

}  /* namespace wire */
}  /* namespace awm */

#endif /* WIRE_UTIL_DEMANGLE_HPP_ */
