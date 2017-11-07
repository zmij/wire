/*
 * ping_pong_options.cpp
 *
 *  Created on: Nov 7, 2017
 *      Author: zmij
 */


#include "ping_pong_options.hpp"

namespace wire {
namespace test {

ping_pong_options&
ping_pong_options::instance()
{
    static ping_pong_options _instance;
    return _instance;
}



} /* namespace test */
} /* namespace wire */
