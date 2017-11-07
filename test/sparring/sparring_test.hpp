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
namespace sparring {

namespace bp = ::boost::process;
namespace bpi = ::boost::process::initializers;
namespace bio = ::boost::iostreams;

class SparringTest: public testing::Test {
public:
    using pipe          = bp::pipe;
    using pipe_end      = bp::pipe_end;
    using pipe_source   = bio::file_descriptor_source;
    using pipe_sink     = bio::file_descriptor_sink;
    using args_type     = ::std::vector< ::std::string >;
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

    asio_config::io_service_ptr io_svc;
    bp::child                    child_;

    virtual void
    SetupArgs(args_type&) = 0;
    virtual void
    ReadSparringOutput(::std::istream&) = 0;

    template < typename T >
    T
    ReadEnpointPort(::std::istream& is)
    {
        T endpoint_data{ "127.0.0.1", 0 };
        is >> endpoint_data.port;
        ::std::cerr << "Sparring partner is listening to "
            << wire::core::detail::endpoint_data_traits< T >::value
            << " port " << endpoint_data.port << "\n";
        return endpoint_data;
    }
private:
    virtual ::std::string const&
    PartnerProgram() const;
private:
    pipe        out_pipe_;
    pipe_source out_source_;
};

} /* namespace sparring */
} /* namespace test */
} /* namespace wire */

#endif /* TRANSPORT_SPARRING_TEST_HPP_ */
