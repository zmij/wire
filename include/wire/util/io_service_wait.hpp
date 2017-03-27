/*
 * io_service_wait.hpp
 *
 *  Created on: Aug 24, 2016
 *      Author: zmij
 */

#ifndef WIRE_UTIL_IO_SERVICE_WAIT_HPP_
#define WIRE_UTIL_IO_SERVICE_WAIT_HPP_

#ifdef WITH_BOOST_FIBERS
#include <wire/util/detail/io_service_wait_fiber.hpp>
#else
#include <wire/util/detail/io_service_wait_thread.hpp>
#endif

#endif /* WIRE_UTIL_IO_SERVICE_WAIT_HPP_ */
