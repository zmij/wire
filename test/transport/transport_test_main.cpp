/*
 * transport_test_main.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */


#include <boost/program_options.hpp>
#include <gtest/gtest.h>

#include "sparring/options.hpp"


// Initialize the test suite
int
main( int argc, char* argv[] )
{
    try {
        namespace po = boost::program_options;
        using wire::test::sparring::options;

        ::testing::InitGoogleTest(&argc, argv);

        po::options_description desc("Test options");
        options& opts = options::instance();

        desc.add_options()
            ("help,h", "show options description")
            ("sparring-partner,s",
                po::value< std::string >(&opts.sparring_partner)->required(),
                "Program to run for server counterpart")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        po::notify(vm);

        return RUN_ALL_TESTS();
    } catch (std::exception const& e) {
        std::cerr << e.what() << "\n";
    }
}

