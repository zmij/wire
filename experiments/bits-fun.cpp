/*
 * bits-fun.cpp
 *
 *  Created on: Dec 14, 2015
 *      Author: zmij
 */

#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdlib>

const int FIRST_COL = 30;

int
main(int argc, char* argv[])
{
	typedef int32_t signed_type;
	typedef std::make_unsigned<signed_type>::type unsigned_type;
	const uint32_t bits_count = sizeof(unsigned_type) * 8 - 1;
	typedef std::bitset< bits_count + 1 > bits_type;
	try {
		if (argc < 2)
			return 1;
		for (int i = 1; i < argc; ++i) {
			signed_type v = std::atoi(argv[i]);
			unsigned_type e =
					static_cast< unsigned_type >((v << 1) ^ (v >> bits_count));

			std::cout << std::setfill(' ')
				<< v << "\n"
				<< std::setw(FIRST_COL) << std::left
					<< "v =" << bits_type(v) << "\n"
				<< std::setw(FIRST_COL) << std::left
					<< "v << 1 =" << bits_type(v << 1) << "\n"
				<< std::setw(FIRST_COL) << std::left
					<< "v >> " << bits_count << " =" << bits_type(v >> bits_count) << "\n"
				<< std::setw(FIRST_COL) << std::left
					<< "(v << 1) ^ (v >> " << bits_count << ") ="
						<< bits_type((v << 1) ^ (v >> bits_count)) << "\n"
			;
			//std::cout << std::setw(FIRST_COL) << std::setfill('*') << '*' << "\n";

			std::cout << std::setfill(' ')
				<< std::setw(FIRST_COL) << std::left
					<< "e << " << bits_count << " =" << bits_type(e << bits_count) << "\n"
				<< std::setw(FIRST_COL) << std::left
					<< "e << " << bits_count << " >> " << bits_count << " ="
					<< bits_type(e << bits_count >> bits_count) << "\n"
				<< std::setw(FIRST_COL) << std::left
					<< "(e >> 1) ^ (e << " << bits_count << " >> "
						<< bits_count << ") ="
					<< bits_type((e >> 1) ^ (e << bits_count >> bits_count)) << "\n"
			;
			std::cout << ((static_cast< signed_type >((e >> 1) ^
					(static_cast<signed_type>(e) << bits_count >> bits_count)) == v) ? "OK" : "FAIL") << "\n";
			std::cout << std::setw(80) << std::setfill('=') << '='
					<< "\n";

		}
	} catch (std::exception const& e) {
		std::cerr << "Exception: " << e.what() << "\n";
		return 1;
	}
	return 0;
}
