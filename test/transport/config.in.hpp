/*
 * config.in.hpp
 *
 *  Created on: 27 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef TRANSPORT_CONFIG_IN_HPP_
#define TRANSPORT_CONFIG_IN_HPP_

#include <string>

namespace wire {
namespace test {

const std::string DATA_ROOT		= "@CMAKE_CURRENT_SOURCE_DIR@";
const std::string CA_ROOT		= "@CMAKE_CURRENT_SOURCE_DIR@/rootCA.crt";
const std::string SERVER_CERT	= "@CMAKE_CURRENT_SOURCE_DIR@/wire-server.crt";
const std::string SERVER_KEY	= "@CMAKE_CURRENT_SOURCE_DIR@/wire-server.key";
const std::string CLIENT_CERT	= "@CMAKE_CURRENT_SOURCE_DIR@/wire-client.crt";
const std::string CLIENT_KEY	= "@CMAKE_CURRENT_SOURCE_DIR@/wire-client.key";

const std::string OTHER_CA		= "@CMAKE_CURRENT_SOURCE_DIR@/otherCA.crt";
const std::string OTHER_CERT	= "@CMAKE_CURRENT_SOURCE_DIR@/wire-other.crt";
const std::string OTHER_KEY		= "@CMAKE_CURRENT_SOURCE_DIR@/wire-other.key";


}  // namespace test
}  // namespace wire


#endif /* TRANSPORT_CONFIG_IN_HPP_ */
