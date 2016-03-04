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

const std::string DATA_SRC_ROOT	= "@CMAKE_CURRENT_SOURCE_DIR@/data";
const std::string DATA_BIN_ROOT	= "@CMAKE_CURRENT_BINARY_DIR@/data";

const std::string CA_ROOT		= DATA_SRC_ROOT + "/rootCA.crt";
const std::string SERVER_CERT	= DATA_SRC_ROOT + "/wire-server.crt";
const std::string SERVER_KEY	= DATA_SRC_ROOT + "/wire-server.key";
const std::string CLIENT_CERT	= DATA_SRC_ROOT + "/wire-client.crt";
const std::string CLIENT_KEY	= DATA_SRC_ROOT + "/wire-client.key";

const std::string OTHER_CA		= DATA_SRC_ROOT + "/otherCA.crt";
const std::string OTHER_CERT	= DATA_SRC_ROOT + "/wire-other.crt";
const std::string OTHER_KEY		= DATA_SRC_ROOT + "/wire-other.key";

const std::string CONNECTOR_CFG	= DATA_BIN_ROOT + "/test_connector_configuration.cfg";

}  // namespace test
}  // namespace wire


#endif /* TRANSPORT_CONFIG_IN_HPP_ */
