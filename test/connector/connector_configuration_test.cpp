/*
 * connector_configuration_test.cpp
 *
 *  Created on: Mar 1, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/core/connector.hpp>
#include <boost/program_options.hpp>
#include "config.hpp"

namespace wire {
namespace core {
namespace test {

TEST(Connector, Configure)
{
	asio_config::io_service_ptr io_service =
			std::make_shared<asio_config::io_service>();
	connector::args_type args{
		"--some.unknown.option",
		"--wire.connector.config_file",
		wire::test::CONNECTOR_CFG
	};
	connector_ptr conn = connector::create_connector( io_service );
	EXPECT_NO_THROW(conn->configure(args));
	EXPECT_THROW(conn->create_adapter("unconfigured"), ::boost::program_options::required_option);
	adapter_ptr adapter;
	EXPECT_NO_THROW(adapter = conn->create_adapter("configured_adapter"));

}

}  // namespace test
}  // namespace core
}  // namespace wire
