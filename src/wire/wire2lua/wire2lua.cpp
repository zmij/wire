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

#include <wire/util/graph.hpp>

#include <wire/wire2lua/lua_generator.hpp>

#include <iterator>
#include <algorithm>

#include <fstream>

namespace wire {
namespace lua {

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

    using global_namespace_list = ::std::vector< ast::global_namespace_ptr >;

    wire::idl::preprocess_options       preproc_opts;
    wire::lua::options                  options;
    wire::idl::lua::generate_options    gen_options;

    po::options_description gen_opts_desc{"General options"};
    gen_opts_desc.add_options()
        ("help,h", "Print help message and exit")
        ("version", "Print version and exit")
        ("preprocess-only,E", "Only run the preprocessor")
    ;

    po::options_description mcpp_opts_desc{"Preprocessor options"};
    mcpp_opts_desc.add_options()
        ("include-dir,I",
            po::value< wire::lua::string_list >(&preproc_opts.include_dirs),
            "Add directory to #include search list")
        ("output-comments,C",
            po::bool_switch(&preproc_opts.output_comments),
            "Output comments from IDL")
    ;
    po::options_description out_opts_desc{"Output options"};
    out_opts_desc.add_options()
        ("output,o",
            po::value< ::std::string >(&gen_options.target_file),
            "Output lua dissector file")
    ;

    po::options_description hidden_opts_desc{"Hidden options"};
    hidden_opts_desc.add_options()
        ("input-file", po::value< wire::lua::string_list >(&options.files),
        "Input file")
    ;

    po::positional_options_description pos_opts_desc;
    pos_opts_desc.add("input-file", -1);

    po::options_description visible_opts{"Command-line options"};
    visible_opts
        .add(gen_opts_desc)
        .add(mcpp_opts_desc)
        .add(out_opts_desc)
    ;

    po::options_description cmd_line_opts{"Command-line options"};
    cmd_line_opts
        .add(gen_opts_desc)
        .add(mcpp_opts_desc)
        .add(out_opts_desc)
        .add(hidden_opts_desc)
    ;

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

    if (gen_options.target_file.empty()) {
        ::std::cerr << "Target file is not specified\n";
        usage(::std::cerr, argv[0], visible_opts);
        return 1;
    }

    if (vm.count("preprocess-only")) {
        for (auto const& file : options.files) {
            preprocessor preproc{ file, preproc_opts };
            ::std::copy( input_stream_iterator{ preproc.stream() },
                    input_stream_iterator{},
                    output_stream_iterator{ ::std::cout } );
        }
    } else {
        ::wire::idl::lua::source_stream ofs{gen_options.target_file};
        if (!ofs) {
            ::std::cerr << "Failed to open output file " << gen_options.target_file;
            return 2;
        }
        global_namespace_list generated;
        // Preprocess and parse
        for (auto const& file : options.files) {
            preprocessor preproc{ file, preproc_opts };

            std::string input_str = preproc.to_string();

            parser::parser p{ input_str };

            auto ns = p.parse();
            generated.push_back(ns);
        }
        // Prepare dependency graph
        ::std::map< ::std::string, global_namespace_list > dependencies;
        for (auto const& g : generated) {
            auto cu = g->current_compilation_unit();
            bool unit_name_printed = false;
            dependencies.emplace(cu->name, global_namespace_list{});
            for (auto d : cu->dependent_units()) {
                auto f = ::std::find_if(generated.begin(), generated.end(),
                    [&](auto ns)
                    {
                        return ns->current_compilation_unit()->name == d->name;
                    });
                if (f != generated.end()) {
                    if (!unit_name_printed) {
                        ::std::cout << cu->name << " depends on:\n";
                        unit_name_printed = true;
                    }
                    ::std::cout << "\t" << d->name << "\n";
                    dependencies[cu->name].push_back(*f);
                }
            }
        }
        global_namespace_list sorted;
        wire::util::graph::topo_sort(
                generated.begin(), generated.end(), ::std::back_inserter(sorted),
                [&](auto ns)
                {
                    auto const& deps = dependencies[ns->current_compilation_unit()->name];
                    return ::std::make_pair(deps.begin(), deps.end());
                });
        for (auto ns : sorted) {
            auto cu = ns->current_compilation_unit();
            ::std::cout << "Generate " << cu->name << " dissector\n";
            lua::generator gen{ns, ofs};
            cu->generate(gen);
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
