/*
 * SparringTest.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#ifndef TRANSPORT_SPARRING_TEST_HPP_
#define TRANSPORT_SPARRING_TEST_HPP_

#include <gtest/gtest.h>

#include <boost/process.hpp>
#include <boost/process/mitigate.hpp>
#include <boost/iostreams/stream.hpp>
#include <wire/asio_config.hpp>

#include <wire/core/endpoint.hpp>

namespace wire {
namespace test {
namespace transport {

namespace bp = boost::process;
namespace bpi = boost::process::initializers;
namespace bio = boost::iostreams;

class SparringTest: public testing::Test {
public:
	typedef bp::pipe					pipe;
	typedef bp::pipe_end				pipe_end;
	typedef bio::file_descriptor_source	pipe_source;
	typedef bio::file_descriptor_sink	pipe_sink;
	typedef std::vector< std::string >	args_type;
public:
	SparringTest();
	virtual ~SparringTest()
	{
	}
protected:
	void
	TearDown() override;

	void
	StartPartner();
	void
	StopPartner();

	bp::child					child_;
	asio_config::io_service_ptr io_svc;

	virtual void
	SetupArgs(args_type&) = 0;
	virtual void
	ReadSparringOutput(std::istream&) = 0;

	template < typename T >
	T
	ReadEnpointPort(std::istream& is)
	{
		T endpoint_data{ "127.0.0.1", 0 };
		is >> endpoint_data.port;
		std::cerr << "Sparring partner is listening to "
			<< wire::core::detail::endpoint_data_traits< T >::value
			<< " port " << endpoint_data.port << "\n";
		return endpoint_data;
	}
private:
	pipe			out_pipe_;
	pipe_source		out_source_;
};

} /* namespace transport */
} /* namespace test */
} /* namespace wire */

#endif /* TRANSPORT_SPARRING_TEST_HPP_ */
