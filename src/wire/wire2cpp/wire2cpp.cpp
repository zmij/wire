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
#include <wire/idl/lexer.hpp>
#include <wire/idl/parser.hpp>

#include <wire/wire2cpp/cpp_generator.hpp>
#include <wire/wire2cpp/fwd_generator.hpp>

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
    using namespace ::wire::idl;

    using input_stream_iterator = ::std::istream_iterator<char>;
    using output_stream_iterator = ::std::ostream_iterator<char>;

    wire::idl::preprocess_options       preproc_opts;
    wire::cpp::options                  options;
    wire::idl::cpp::generate_options    gen_options;

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
        ("output-comments,C",
            po::bool_switch(&preproc_opts.output_comments),
            "Output comments from IDL")
        ("verbose,v",
            po::bool_switch(&preproc_opts.verbose),
            "Verbose output")
    ;
    po::options_description out_opts_desc{"Output options"};
    out_opts_desc.add_options()
        ("header-output-dir,o",
            po::value< ::std::string >(&gen_options.header_output_dir),
            "Directory to place generated headers")
        ("cpp-output-dir,c",
            po::value< ::std::string >(&gen_options.source_output_dir),
            "Directory to place generated source files")
        ("header-include-root,i",
            po::value< ::std::string >(&gen_options.header_include_dir),
            "Root path of include files, for calculating a relative path for inclusion")
        ("dont-use-hashed-ids,n",
            po::bool_switch(&gen_options.dont_use_hashed_id)->default_value(false),
            "Always use string type id in class and exception marshaling")
        ("dates",
            po::value< cpp::date_and_time >(&cpp::generator::datetime_type)
                ->default_value(cpp::date_and_time::std),
            "Which mapping for date and time types to use (std::chrono or boost::posix_time types)")
        ("generate-forwards,f",
            po::bool_switch(&gen_options.generate_forwards)->default_value(false),
            "Generate a header with forward declarations")
    ;

    po::options_description generate_opts_desc{"Generate options"};
    generate_opts_desc.add_options()
        ("generate-plugin",
            po::value< ::wire::idl::cpp::generate_options::string_list >(&gen_options.plugins),
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
        preprocessor preproc{ file, preproc_opts };

        if (vm.count("preprocess-only")) {
            ::std::copy( input_stream_iterator{ preproc.stream() },
                    input_stream_iterator{},
                    output_stream_iterator{ ::std::cout } );
        } else {
            std::string input_str = preproc.to_string();

            parser::parser p{ input_str };

            auto ns = p.parse();
            auto cu = ns->current_compilation_unit();
            wire::idl::cpp::generator gen(gen_options, preproc_opts, ns);
            cu->generate(gen);
            if (gen_options.generate_forwards) {
                ::wire::idl::cpp::fwd_generator fgen(gen_options, preproc_opts, ns);
                cu->generate(fgen);
            }
        }
    }

    return 0;
} catch (::wire::idl::syntax_error const& se) {
    ::std::cerr << se.what() << "\n";
    return 1;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 2;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 3;
}
