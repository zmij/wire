/*
 * locator_test_main.cpp
 *
 *  Created on: Sep 29, 2016
 *      Author: zmij
 */


#include <boost/program_options.hpp>
#include <gtest/gtest.h>


// Initialize the test suite
int
main( int argc, char* argv[] )
try {
    namespace po = boost::program_options;
    ::testing::InitGoogleTest(&argc, argv);

    po::options_description desc("Test options");

    desc.add_options()
            ("help,h", "show options description")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    po::notify(vm);

    return RUN_ALL_TESTS();
} catch (::std::exception const& e) {
    ::std::cerr << e.what() << "\n";
}
