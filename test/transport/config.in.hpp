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
const std::string CA_ROOT		= "rootCA.crt";
const std::string SERVER_CERT	= "wire-server.crt";
const std::string SERVER_KEY	= "wire-server.key";
const std::string CLIENT_CERT	= "wire-client.crt";
const std::string CLIENT_KEY	= "wire-client.key";

}  // namespace test
}  // namespace wire


#endif /* TRANSPORT_CONFIG_IN_HPP_ */
