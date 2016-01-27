/*
 * sparring_options.cpp
 *
 *  Created on: 27 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#include "sparring_options.hpp"

namespace wire {
namespace test {

sparring_options&
sparring_options::instance()
{
	static sparring_options opts_;
	return opts_;
}

} /* namespace test */
} /* namespace wire */
