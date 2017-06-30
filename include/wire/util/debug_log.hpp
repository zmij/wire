/*
 * debug_log.hpp
 *
 *  Created on: Jun 30, 2017
 *      Author: zmij
 */

#ifndef WIRE_UTIL_DEBUG_LOG_HPP_
#define WIRE_UTIL_DEBUG_LOG_HPP_

#ifdef DEBUG_OUTPUT
#include <iostream>
#include <sstream>
#endif

#ifdef DEBUG_OUTPUT

#define WRITE_DEBUG_LOG(...) \
        { ::std::ostringstream os;\
        os << ::getpid() << " " << __VA_ARGS__ << "\n";\
        ::std::cerr << os.str(); }
#define WRITE_DEBUG_LOG_TAG(tag, ...) \
        { ::std::ostringstream os;\
        tag(os) << " " << __VA_ARGS__ << "\n";\
        ::std::cerr << os.str(); }

#if DEBUG_OUTPUT >= 5

#define DEBUG_LOG1(F, ...) F(__VA_ARGS__)
#define DEBUG_LOG2(F, ...) F(__VA_ARGS__)
#define DEBUG_LOG3(F, ...) F(__VA_ARGS__)
#define DEBUG_LOG4(F, ...) F(__VA_ARGS__)
#define DEBUG_LOG5(F, ...) F(__VA_ARGS__)

#elif DEBUG_OUTPUT == 4

#define DEBUG_LOG1(F, ...) F(__VA_ARGS__)
#define DEBUG_LOG2(F, ...) F(__VA_ARGS__)
#define DEBUG_LOG3(F, ...) F(__VA_ARGS__)
#define DEBUG_LOG4(F, ...) F(__VA_ARGS__)
#define DEBUG_LOG5(...)

#elif DEBUG_OUTPUT == 3

#define DEBUG_LOG1(F, ...) F(__VA_ARGS__)
#define DEBUG_LOG2(F, ...) F(__VA_ARGS__)
#define DEBUG_LOG3(F, ...) F(__VA_ARGS__)
#define DEBUG_LOG4(...)
#define DEBUG_LOG5(...)

#elif DEBUG_OUTPUT == 2

#define DEBUG_LOG1(F, ...) F(__VA_ARGS__)
#define DEBUG_LOG2(F, ...) F(__VA_ARGS__)
#define DEBUG_LOG3(...)
#define DEBUG_LOG4(...)
#define DEBUG_LOG5(...)

#elif DEBUG_OUTPUT == 1

#define DEBUG_LOG1(F, ...) F(__VA_ARGS__)
#define DEBUG_LOG2(...)
#define DEBUG_LOG3(...)
#define DEBUG_LOG4(...)
#define DEBUG_LOG5(...)

#endif

#else
#define DEBUG_LOG1(...)
#define DEBUG_LOG2(...)
#define DEBUG_LOG3(...)
#define DEBUG_LOG4(...)
#define DEBUG_LOG5(...)
#endif

#define DEBUG_LOG_LEVEL(N) DEBUG_LOG##N
#define DEBUG_LOG(N, ...) DEBUG_LOG_LEVEL(N)(WRITE_DEBUG_LOG, __VA_ARGS__)
#define DEBUG_LOG_TAG(N, ...) DEBUG_LOG_LEVEL(N)(WRITE_DEBUG_LOG_TAG, __VA_ARGS__)

#endif /* WIRE_UTIL_DEBUG_LOG_HPP_ */
