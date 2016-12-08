/*
 * codepoints.cpp
 *
 *  Created on: Dec 8, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <codecvt>

namespace wire {
namespace json {
namespace test {

TEST(Codepoint, Convert)
{
    ::std::string expected = u8"\u00a9";
    ::std::cerr << expected << "\n";

    ::std::wstring_convert<::std::codecvt_utf8<char32_t>, char32_t> converter;
    ::std::string test = converter.to_bytes(0x00a9);
    ::std::cerr << test << "\n";

    ::std::wstring_convert<::std::codecvt_utf8<wchar_t>> wconverter;
    ::std::wstring wtest = wconverter.from_bytes(test);
    ::std::wcerr << wtest << L"\n";
}

}  /* namespace test */
}  /* namespace json */
}  /* namespace wire */
