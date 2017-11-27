/*
 * varint-decode.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: zmij
 */

#include <iostream>
#include <sstream>
#include <iomanip>
#include <iterator>
#include <vector>
#include <string>
#include <bitset>

#include <boost/program_options.hpp>
#include <boost/endian/arithmetic.hpp>
#include <wire/encoding/wire_io.hpp>

typedef std::vector< std::string > string_list;
typedef std::bitset<8> binary_byte;

struct options {
	bool		unsigned_;
	string_list	inputs;
	bool        bits    = false;
};

uint8_t
nibble_val(char n)
{
	if ('0' <= n && n <= '9') {
		return n - '0';
	}
	if ('a' <= n && n <= 'f') {
		return n - 'a' + 10;
	}
	if ('A' <= n && n <= 'F') {
		return n - 'A' + 10;
	}
	throw std::runtime_error("Invalid hex digit");
}

uint8_t
str_to_byte(char hi, char lo)
{
	return (nibble_val(hi) << 4) | nibble_val(lo);
}

int
main(int argc, char* argv[])
{
	namespace po = boost::program_options;
	try {
		options opts;
		po::options_description desc("Varint decoding options");
		desc.add_options()
			("unsigned,u",      po::bool_switch(&opts.unsigned_), "Decode as unsigned")
			("input-string",    po::value<string_list>(&opts.inputs), "Input strings")
			("output-bits,b",   po::bool_switch(&opts.bits),        "Output bit stream")
		;
		po::positional_options_description pos;
		pos.add("input-string", -1);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).
		          options(desc).positional(pos).run(), vm);
		po::notify(vm);

		for (auto s : opts.inputs) {
			std::istringstream is(s);
			uint8_t val;
			char lo, hi;
			std::vector<uint8_t> buffer;
			while (is.get(hi)) {
				if (!is.get(lo)) break;
				val = str_to_byte(hi, lo);
                if (opts.bits)
				    std::cout << binary_byte(val) << " ";
				buffer.push_back(val);
			}
            if (opts.bits)
                std::cout << "\n";

			auto b = buffer.begin();
			auto e = buffer.end();
            auto p = b;
			if (opts.unsigned_) {
				uint64_t v;
				wire::encoding::read(b, e, v);
				std::cout << "Dec: " << v << "\n"
				          << "Hex: 0x" << ::std::hex << v << "\n";
			} else {
				int64_t v;
				wire::encoding::read(b, e, v);
				std::cout << "Dec: " << v << "\n"
                          << "Hex: 0x" << ::std::hex << v << "\n";
			}
            std::cout << "Bytes used " << ::std::dec << (b - p) << "\n";
		}
	} catch (std::exception const& e) {
		std::cerr << "Exception: " << e.what() << "\n";
		return 1;
	}
	return 0;
}
