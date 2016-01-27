/*
 * options.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#include "options.hpp"

namespace wire {
namespace test {
namespace transport {

options&
options::instance()
{
	static options opts_;
	return opts_;
}

} /* namespace transport */
} /* namespace test */
} /* namespace wire */
