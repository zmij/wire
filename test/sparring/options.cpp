/*
 * options.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#include "options.hpp"

namespace wire {
namespace test {
namespace sparring {

options&
options::instance()
{
    static options opts_;
    return opts_;
}

} /* namespace sparring */
} /* namespace test */
} /* namespace wire */
