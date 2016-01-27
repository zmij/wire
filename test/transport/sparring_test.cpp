/*
 * SparringTest.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#include "sparring_test.hpp"
#include "options.hpp"

namespace wire {
namespace test {
namespace transport {

SparringTest::SparringTest()
	: ::testing::Test(),
		out_pipe_(bp::create_pipe()),
		out_source_(out_pipe_.source, bio::close_handle),
		child_ { 0 }
{
}

void
SparringTest::SetUp()
{
	{
		args_type args {
			wire::test::transport::options::instance().sparring_partner
		};
		SetupArgs(args);
		pipe_sink out_sink(out_pipe_.sink, bio::close_handle);
		child_ = bp::execute(
			bpi::set_args(args),
			bpi::bind_stdout(out_sink),
			bpi::inherit_env(),
			bpi::throw_on_error()
		);
	}
	bio::stream< pipe_source > is(out_source_);
	ReadSparringOutput(is);
}

void
SparringTest::TearDown()
{
	if (child_.pid)
		::kill(child_.pid, SIGTERM);
}

void
SparringTest::StopPartner()
{
	if (child_.pid) {
		::kill(child_.pid, SIGTERM);
		child_.pid = 0;
	}
}

} /* namespace transport */
} /* namespace test */
} /* namespace wire */
