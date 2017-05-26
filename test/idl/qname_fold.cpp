/*
 * qname_fold.cpp
 *
 *  Created on: May 26, 2017
 *      Author: zmij
 */

#include <iostream>
#include <wire/idl/qname.hpp>

int
main(int argc, char* argv[])
try {
    using namespace ::wire::idl::ast;
    if (argc < 3) {
        ::std::cerr << "Not enough arguments\n";
        return 1;
    }
    qname scope{argv[1]};
    qname name{argv[2]};

    ::std::cout << name.in_scope(scope) << "\n";
    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
