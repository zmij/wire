/*
 * preprocess.cpp
 *
 *  Created on: 16 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/idl/preprocess.hpp>
#include <wire/idl/config.hpp>
#include <boost/process.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <iterator>
#include <algorithm>
#include <iostream>

namespace wire {
namespace ast {

namespace bp = ::boost::process;
namespace bi = ::boost::process::initializers;
namespace io = ::boost::iostreams;

struct preprocessor::impl {
    using pipe_source  = io::file_descriptor_source;
    using pipe_istream = io::stream< io::file_descriptor_source >;

    bp::pipe        pipe_;
    pipe_source     source_;
    pipe_istream    istream_;

    impl(::std::string const& file_name, preprocess_options const& options)
        : pipe_{ bp::create_pipe() },
          source_{ pipe_.source, io::close_handle },
          istream_{ source_ }
    {
        ::std::noskipws(istream_);

        string_list args{ MCPP_PATH, "-+", "-DWIRE_GENERATE" };
        for (auto const& p : options.include_dirs) {
            args.push_back("-I");
            args.push_back(p);
        }
        args.push_back(file_name);

        ::std::cerr << "Execute preprocessor";
        for (auto const& a : args) {
            ::std::cerr << " " << a;
        }
        ::std::cerr << "\n";

        io::file_descriptor_sink sink{ pipe_.sink, io::close_handle };

        bp::child p = bp::execute(
            bi::set_args(args),
            bi::bind_stdout{sink},
            bi::throw_on_error{}
        );
        int status = bp::wait_for_exit(p);
        if (status != 0) {
            throw ::std::runtime_error("Failed to preprocess file " + file_name);
        }
    }
};

preprocessor::preprocessor(::std::string const& file_name, preprocess_options const& options)
    : pimpl_{ ::std::make_shared<impl>( file_name, options ) }
{
}

::std::istream&
preprocessor::stream()
{
    return pimpl_->istream_;
}

}  /* namespace ast */
}  /* namespace wire */
