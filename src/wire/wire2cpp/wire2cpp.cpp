/*
 * wire2cpp.cpp
 *
 *  Created on: 16 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <iostream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <wire/idl/preprocess.hpp>

#include <iterator>
#include <algorithm>

namespace wire {
namespace cpp {

using string_list = ::std::vector< ::std::string >;

struct options {
    string_list    files;
};

}  // namespace cpp
}  // namespace wire

void
usage(::std::ostream& os, char const * prog_name,
    ::boost::program_options::options_description const& opts)
{
    os << "Usage:\n" << prog_name << " [options] file [... file]\n" << opts << "\n";
}

int
main(int argc, char* argv[])
try {
    namespace po = boost::program_options;
    using input_stream_iterator = ::std::istream_iterator<char>;
    using output_stream_iterator = ::std::ostream_iterator<char>;

    wire::ast::preprocess_options   preproc_opts;
    wire::cpp::options              options;

    po::options_description gen_opts_desc{"General options"};
    gen_opts_desc.add_options()
        ("help,h", "Print help message and exit")
        ("version", "Print version and exit")
        ("preprocess-only,E", "Only run the preprocessor")
    ;

    po::options_description mcpp_opts_desc{"Preprocessor options"};
    mcpp_opts_desc.add_options()
        ("include-dir,I",
            po::value< wire::cpp::string_list >(&preproc_opts.include_dirs),
            "Add directory to #include search list")
    ;
    po::options_description out_opts_desc{"Output options"};
    out_opts_desc.add_options()
        ("header-output-dir",
            "Directory to place generated headers")
        ("header-include-root",
            "Root path of include files, for calculating a relative path for inclusion")
        ("cpp-output-dir",
            "Directory to place generated source files")
    ;

    po::options_description generate_opts_desc{"Generate options"};
    generate_opts_desc.add_options()
        ("generate-plugin",
            "Use a plugin for code generation, format: library_name:entry_point")
    ;

    po::options_description hidden_opts_desc{"Hidden options"};
    hidden_opts_desc.add_options()
        ("input-file", po::value< wire::cpp::string_list >(&options.files),
        "Input file")
    ;

    po::positional_options_description pos_opts_desc;
    pos_opts_desc.add("input-file", -1);

    po::options_description visible_opts{"Command-line options"};
    visible_opts
        .add(gen_opts_desc)
        .add(mcpp_opts_desc)
        .add(out_opts_desc)
        .add(generate_opts_desc);

    po::options_description cmd_line_opts{"Command-line options"};
    cmd_line_opts
        .add(gen_opts_desc)
        .add(mcpp_opts_desc)
        .add(out_opts_desc)
        .add(generate_opts_desc)
        .add(hidden_opts_desc);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv)
        .positional(pos_opts_desc)
        .options(cmd_line_opts).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        usage(::std::cout, argv[0], visible_opts);
        return 0;
    }

    if (!vm.count("input-file")) {
        ::std::cerr << "No input files given\n";
        usage(::std::cerr, argv[0], visible_opts);
        return 1;
    }

    for (auto const& file : options.files) {
        ::std::cerr << "Process " << file << "\n";
        wire::ast::preprocessor preproc{ file, preproc_opts };

        if (vm.count("preprocess-only")) {
            ::std::copy( input_stream_iterator{ preproc.stream() },
                    input_stream_iterator{},
                    output_stream_iterator{ ::std::cout } );
        } else {
            ::std::cerr << "Generate files here\n";
        }
    }

    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 1;
}
