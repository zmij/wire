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
	SetUp() override;
	void
	TearDown() override;

	void
	StopPartner();

	bp::child		child_;

	virtual void
	SetupArgs(args_type&) = 0;
	virtual void
	ReadSparringOutput(std::istream&) = 0;
private:
	pipe			out_pipe_;
	pipe_source		out_source_;
};

} /* namespace transport */
} /* namespace test */
} /* namespace wire */

#endif /* TRANSPORT_SPARRING_TEST_HPP_ */
