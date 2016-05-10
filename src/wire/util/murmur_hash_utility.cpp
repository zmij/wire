/*
 * murmur_hash_utility.cpp
 *
 *  Created on: 10 мая 2016 г.
 *      Author: sergey.fedorov
 */

#include <iostream>
#include <sstream>
#include <iomanip>

#include <wire/util/murmur_hash.hpp>

int
main(int argc, char* argv[])
try {
    ::std::ostringstream os;
    for (int i = 1; i < argc; ++i) {
        os << argv[i];
    }
    ::std::cout << "0x" << ::std::hex << wire::hash::murmur_hash(os.str()) << "\n";
    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Standard exception " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
