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
#include <sstream>

namespace wire {
namespace idl {

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
        if (options.output_comments) {
            args.push_back("-C");
        }
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

    ::std::string
    to_string()
    {
        using input_stream_iterator = ::std::istream_iterator<char>;
        using output_stream_iterator = ::std::ostream_iterator<char>;
        ::std::ostringstream os;

        ::std::copy( input_stream_iterator{ istream_ },
                input_stream_iterator{},
                output_stream_iterator{ os } );

        return ::std::move(os.str());
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

::std::string
preprocessor::to_string()
{
    return pimpl_->to_string();
}

}  /* namespace ast */
}  /* namespace wire */
