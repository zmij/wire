/*
 * connector_configuration_test.cpp
 *
 *  Created on: Mar 1, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/core/connector.hpp>

namespace wire {
namespace core {
namespace test {

TEST(Connector, Configure)
{
	asio_config::io_service_ptr io_service =
			std::make_shared<asio_config::io_service>();
	connector::args_type args{
		"--some.unknown.option"
	};
	connector_ptr conn = connector::create_connector( io_service );
	EXPECT_NO_THROW(conn->configure(args));
}

}  // namespace test
}  // namespace core
}  // namespace wire
