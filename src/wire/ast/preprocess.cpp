/*
 * preprocess.cpp
 *
 *  Created on: 16 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/ast/preprocess.hpp>
#include <wire/ast/config.hpp>
#include <boost/process.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <iterator>
#include <algorithm>
#include <iostream>

namespace wire {
namespace ast {

void
preprocess(::std::string const& file_name, ::std::ostream& os,
        preprocess_options const& options)
{
    namespace bp = ::boost::process;
    namespace bi = ::boost::process::initializers;
    namespace io = ::boost::iostreams;

    using input_iterator = ::std::istream_iterator< char >;
    using output_iterator = ::std::ostream_iterator< char >;

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

    bp::pipe p = bp::create_pipe();
    {
        io::file_descriptor_sink sink{ p.sink, io::close_handle };

        bp::execute(
            bi::set_args(args),
            bi::bind_stdout{sink},
            bi::throw_on_error{}
        );
    }

    io::file_descriptor_source source{ p.source, io::close_handle };
    io::stream< io::file_descriptor_source > is(source);
    ::std::noskipws(is);

    ::std::copy(input_iterator{is}, input_iterator{}, output_iterator{os});
}


}  /* namespace ast */
}  /* namespace wire */
