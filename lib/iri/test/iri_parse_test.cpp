/*
 * grammar_test.cpp
 *
 *  Created on: Aug 12, 2015
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <grammar/grammar_parse_test.hpp>
#include <tip/iri/grammar/iri_parse.hpp>

GRAMMAR_TEST(tip::iri::grammar::parse::sub_delims_grammar, SubDelims,
		::testing::Values("!", "$", "&", "'", "(", ")", "*", "+", ",", ";", "="),
		::testing::Values(":", "a", "4", "±", "§", "-", "_")
);

GRAMMAR_TEST(tip::iri::grammar::parse::gen_delims_grammar, GenDelims,
		::testing::Values(":", "/", "?", "#", "[", "]", "@"),
		::testing::Values(";", "a", "4", "±", "§", "-")
);

GRAMMAR_TEST(tip::iri::grammar::parse::reserved_grammar, Reserved,
		::testing::Values(":", "/", "?", "#", "[", "]", "@",
				"!", "$", "&", "'", "(", ")", "*", "+", ",", ";", "="),
		::testing::Values("~", "a", "4", "±", "§", ".")
);

GRAMMAR_TEST(tip::iri::grammar::parse::unreserved_grammar, Unreserved,
		::testing::Values("~", "a", "4", "z", "R", ".", "-", "_"),
		::testing::Values(":", "/", "?", "#", "[", "]", "@",
				"!", "$", "&", "'", "(", ")", "*", "+", ",", ";", "=")
);

GRAMMAR_TEST(tip::iri::grammar::parse::pct_encoded_grammar, PctEncoded,
		::testing::Values("%00", "%af", "%f0", "%bd", "%9c"),
		::testing::Values("%fh", "%0", "%fda", "%900", "%zx", "%as")
);

GRAMMAR_TEST(tip::iri::grammar::parse::iprivate_grammar, Private,
		::testing::Values("%xe000", "%xe001", "%xf0da", "%xf0000", "%x10fffd"),
		::testing::Values("%fh", "%0", "%fda", "%900", "%zx",
				"%xdfff", "%xf900")
);

GRAMMAR_TEST(tip::iri::grammar::parse::ucschar_grammar, UcsChar,
		::testing::Values("%xA0", "%x10fa", "%xd7ff",
				"%xf900", "%xf901", "%xfd1a", "%xfdcf",
				"%xfdf0", "%xffef", "%x10000", "%x1fffd",
				"%x20000", "%x2fffd", "%x30000", "%x3fffd",
				"%x40000", "%x4fffd", "%x50000", "%x6fffd",
				"%xe1000", "%xefffd"
		),
		::testing::Values("%xe000", "%xe001", "%xf0da", "%xf0000", "%x10fffd",
				"%xd800", "%xfdef", "%xefffe")
);

// TODO iunreserved test
// TODO ipchar test
// TODO iquery test
// TODO ifragment test
// TODO isegment* test

GRAMMAR_TEST(tip::iri::grammar::parse::scheme_grammar, Scheme,
	::testing::Values(
		"http", "https", "ftp", "mailto"
	),
	::testing::Values(
		"127", "-ht", "ht_tp"
	)
);

template < typename InputIterator >
using ipv4_grammar_int = tip::iri::grammar::parse::ipv4_grammar< InputIterator, std::uint32_t()>;

GRAMMAR_TEST(ipv4_grammar_int, IP4,
		::testing::Values(
			"127.0.0.1",
			"255.255.2.1",
			"10.0.254.1",
			"192.0.2.33"
		),
		::testing::Values(
			"127.0.0.256",
			"255.255.2.387",
			"400.0.254.1",
			"192.0..33"
		)
);

template < typename InputIterator >
using ipv4_grammar_str = tip::iri::grammar::parse::ipv4_grammar< InputIterator, std::string()>;

GRAMMAR_TEST(ipv4_grammar_str, IP4Str,
	::testing::Values(
		"127.0.0.1",
		"255.255.2.1",
		"10.0.254.1",
		"192.0.2.33"
	),
	::testing::Values(
		"127.0.0.256",
		"255.255.2.387",
		"400.0.254.1",
		"192.0..33"
	)
);

GRAMMAR_PARSE_TEST(ipv4_grammar_str, IP4, std::string,
	::testing::Values(
		ParseIP4::make_test_data("127.0.0.1", "127.0.0.1"),
		ParseIP4::make_test_data("255.255.2.1", "255.255.2.1")
	)
);

template < typename InputIterator >
using ipv6_grammar_int =
		tip::iri::grammar::parse::ipv6_grammar< InputIterator, std::array< std::uint16_t, 8 >() >;


GRAMMAR_TEST(ipv6_grammar_int, IP6,
	::testing::Values(
		"1:2:3:4:5:6:7:8",
		"1::",												"1:2:3:4:5:6:7::",
		"1::8",					"1:2:3:4:5:6::8",			"1:2:3:4:5:6::8",
		"1::7:8",				"1:2:3:4:5::7:8",			"1:2:3:4:5::8",
		"1::6:7:8",				"1:2:3:4::6:7:8",			"1:2:3:4::8",
		"1::5:6:7:8",			"1:2:3::5:6:7:8",			"1:2:3::8",
		"1::4:5:6:7:8",			"1:2::4:5:6:7:8",			"1:2::8",
		"1::3:4:5:6:7:8",		"1::3:4:5:6:7:8",			"1::8",
		"::2:3:4:5:6:7:8",		"::2:3:4:5:6:7:8",			"::8",	"::",

		"::192.0.2.33",
		"1::192.0.2.33",
		"1:2::192.0.2.33",
		"1:2:3::192.0.2.33",
		"1:2:3:4::192.0.2.33",
		"1:2:3:4:5::192.0.2.33",
		"1:2:3:4:5:6:192.0.2.33",
		"1:2:3:4:5::192.0.2.33",
		"1:2:3:4::6:192.0.2.33",
		"1:2:3::5:6:192.0.2.33",
		"1:2::4:5:6:192.0.2.33",
		"1::3:4:5:6:192.0.2.33",
		"::2:3:4:5:6:192.0.2.33"
	),
	::testing::Values(
		"127.0.0.256",
		"255.255.2.387",
		"400.0.254.1",
		"192.0..33",
		"1:::3",
		":::1:3:4",
		"1:2:3:::"
	)
);

GRAMMAR_TEST(ipv6_grammar_int, IP6_2,
	::testing::Values(
		"1:2:3:4:5:6:7:8",
		"1:2:3:4:5:6:7::",

		"1:2:3:4:5:6:255.255.255.255",
		"1:2:3:4:5:6::8",
		"1:2:3:4:5:6::",

		"1:2:3:4:5::255.255.255.255",
		"1:2:3:4:5::7:8",
		"1:2:3:4:5::8",
		"1:2:3:4:5::",

		"1:2:3:4::6:255.255.255.255",	// full match
		"1:2:3:4::255.255.255.255",
		"1:2:3:4::6:7:8",				// full match
		"1:2:3:4::7:8",
		"1:2:3:4::8",					// full match
		"1:2:3:4::",					// full match

		"1:2:3::5:6:255.255.255.255",	// full match
		"1:2:3::6:255.255.255.255",
		"1:2:3::255.255.255.255",
		"1:2:3::5:6:7:8",				// full match
		"1:2:3::6:7:8",
		"1:2:3::7:8",
		"1:2:3::8",						// full match
		"1:2:3::"						// full match
	),
	::testing::Values( "127.0.0.256", "255.255.2.387" )
);

GRAMMAR_TEST(ipv6_grammar_int, IP6_3,
	::testing::Values(
		"1:2::4:5:6:255.255.255.255",
		"1:2::5:6:255.255.255.255",
		"1:2::6:255.255.255.255",
		"1:2::255.255.255.255",
		"1:2::4:5:6:7:8",
		"1:2::5:6:7:8",
		"1:2::6:7:8",
		"1:2::7:8",
		"1:2::8",
		"1:2::",

		"1::3:4:5:6:255.255.255.255",
		"1::4:5:6:255.255.255.255",
		"1::5:6:255.255.255.255",
		"1::6:255.255.255.255",
		"1::255.255.255.255",
		"1::3:4:5:6:7:8",
		"1::4:5:6:7:8",
		"1::5:6:7:8",
		"1::6:7:8",
		"1::7:8",
		"1::8",
		"1::",

		"::2:3:4:5:6:255.255.255.255",
		"::3:4:5:6:255.255.255.255",
		"::4:5:6:255.255.255.255",
		"::5:6:255.255.255.255",
		"::6:255.255.255.255",
		"::255.255.255.255",
		"::2:3:4:5:6:7:8",
		"::3:4:5:6:7:8",
		"::4:5:6:7:8",
		"::5:6:7:8",
		"::6:7:8",
		"::7:8",
		"::8",
		"::"
	),
	::testing::Values( "127.0.0.256", "255.255.2.387" )
);

typedef std::array< std::uint16_t, 8> ipv6;
GRAMMAR_PARSE_TEST(ipv6_grammar_int, IP6, ipv6,
		::testing::Values(
			ParseIP6::make_test_data("1:2:3:4:5:6:7:8", {1, 2, 3, 4, 5, 6, 7, 8}),
			ParseIP6::make_test_data("1:2:3:4:5:6:7::", {1, 2, 3, 4, 5, 6, 7, 0}),
			ParseIP6::make_test_data("1:2:3:4:5:6::8", {1, 2, 3, 4, 5, 6, 0, 8}),
			ParseIP6::make_test_data("1:2:3:4:5::7:8", {1, 2, 3, 4, 5, 0, 7, 8}),
			ParseIP6::make_test_data("1:2:3:4::6:7:8", {1, 2, 3, 4, 0, 6, 7, 8}),
			ParseIP6::make_test_data("1:2:3::5:6:7:8", {1, 2, 3, 0, 5, 6, 7, 8}),
			ParseIP6::make_test_data("1:2::4:5:6:7:8", {1, 2, 0, 4, 5, 6, 7, 8}),
			ParseIP6::make_test_data("1::3:4:5:6:7:8", {1, 0, 3, 4, 5, 6, 7, 8}),
			ParseIP6::make_test_data("::2:3:4:5:6:7:8", {0, 2, 3, 4, 5, 6, 7, 8}),

			ParseIP6::make_test_data("::8", {0, 0, 0, 0, 0, 0, 0, 8}),
			ParseIP6::make_test_data("1::", {1, 0, 0, 0, 0, 0, 0, 0}),
			ParseIP6::make_test_data("1::8", {1, 0, 0, 0, 0, 0, 0, 8})
		)
);

template < typename InputIterator >
using ipv6_grammar_str =
		tip::iri::grammar::parse::ipv6_grammar< InputIterator, std::string() >;

GRAMMAR_TEST(ipv6_grammar_str, IP6Str,
	::testing::Values(
		"1:2::4:5:6:255.255.255.255",
		"1:2::5:6:255.255.255.255",
		"1:2::6:255.255.255.255",
		"1:2::255.255.255.255",
		"1:2::4:5:6:7:8",
		"1:2::5:6:7:8",
		"1:2::6:7:8",
		"1:2::7:8",
		"1:2::8",
		"1:2::",

		"1::3:4:5:6:255.255.255.255",
		"1::4:5:6:255.255.255.255",
		"1::5:6:255.255.255.255",
		"1::6:255.255.255.255",
		"1::255.255.255.255",
		"1::3:4:5:6:7:8",
		"1::4:5:6:7:8",
		"1::5:6:7:8",
		"1::6:7:8",
		"1::7:8",
		"1::8",
		"1::",

		"::2:3:4:5:6:255.255.255.255",
		"::3:4:5:6:255.255.255.255",
		"::4:5:6:255.255.255.255",
		"::5:6:255.255.255.255",
		"::6:255.255.255.255",
		"::255.255.255.255",
		"::2:3:4:5:6:7:8",
		"::3:4:5:6:7:8",
		"::4:5:6:7:8",
		"::5:6:7:8",
		"::6:7:8",
		"::7:8",
		"::8",
		"::"
	),
	::testing::Values( "127.0.0.256", "255.255.2.387" )
);

GRAMMAR_PARSE_TEST(ipv6_grammar_str, IP6Str, std::string,
		::testing::Values(
			ParseIP6Str::make_test_data("1:2:3:4:5:6:7:8", "1:2:3:4:5:6:7:8"),
			ParseIP6Str::make_test_data("1:2:3:4:5:6:7::", "1:2:3:4:5:6:7::"),
			ParseIP6Str::make_test_data("1:2:3:4:5:6::8", "1:2:3:4:5:6::8"),
			ParseIP6Str::make_test_data("::2:3:4:5:6:255.255.255.255", "::2:3:4:5:6:255.255.255.255")
		)
);

GRAMMAR_TEST(tip::iri::grammar::parse::ipath_abempty_grammar, IPathAbempty,
	::testing::Values(
		"", "/bla", "/flbla/asdf"
	),
	::testing::Values(
		"127", "-ht", "ht_tp"
	)
);

GRAMMAR_TEST(tip::iri::grammar::parse::ipath_absolute_grammar, IPathAbsolute,
	::testing::Values(
		"/", "/bla", "/flbla/asdf"
	),
	::testing::Values(
		"127", "-ht", "ht_tp", ""
	)
);

GRAMMAR_TEST(tip::iri::grammar::parse::ipath_noscheme_grammar, IPathNoscheme,
	::testing::Values(
		"bla", "flbla/a:sdf"
	),
	::testing::Values(
		"", "/", "foo:bar"
	)
);

GRAMMAR_TEST(tip::iri::grammar::parse::ipath_rootless_grammar, IPathRootless,
	::testing::Values(
		"bla", "foo:bar/a:sdf"
	),
	::testing::Values(
		"", "/", "/adfsd"
	)
);

GRAMMAR_TEST(tip::iri::grammar::parse::ipath_grammar, IPath,
	::testing::Values(
		"bla", "foo:bar/a:sdf", "bla", "foo/a:sdf",
		"", "/bla", "/flbla/asdf", "/"
	),
	::testing::Values(
		"±", "§", "`", "?asdf", "%zxcv"
	)
);

GRAMMAR_PARSE_TEST(tip::iri::grammar::parse::ipath_grammar, Path, tip::iri::path,
	::testing::Values(
		ParsePath::make_test_data("bla", tip::iri::path{false, {"bla"} }),
		ParsePath::make_test_data("foo:bar/a:sdf",
				tip::iri::path{false, {"foo:bar", "a:sdf"}}),
		ParsePath::make_test_data("foo/a:sdf",
				tip::iri::path{false, {"foo", "a:sdf"}}),
		ParsePath::make_test_data("/", tip::iri::path{true}),
		ParsePath::make_test_data("/foo/bar",
				tip::iri::path{true, {"foo", "bar"}})
	)
);

GRAMMAR_PARSE_TEST(tip::iri::grammar::parse::ip_literal_grammar, IPLiteral, std::string,
	::testing::Values(
		ParseIPLiteral::make_test_data("[ffaa:1234::aadd:255.255.255.0]",
				"[ffaa:1234::aadd:255.255.255.0]"),
		ParseIPLiteral::make_test_data("[ffaa:1234::aadd:dfbc]",
				"[ffaa:1234::aadd:dfbc]")
	)
);


GRAMMAR_TEST(tip::iri::grammar::parse::ihost_grammar, IHost,
	::testing::Values(
		"google.com", "www.mail.ru", "[::8]", "127.0.0.1", "[ffaa:1234::aadd:255.255.255.0]"
	),
	::testing::Values(
		"±", "§", "`", "?asdf", "%zxcv"
	)
);

GRAMMAR_PARSE_TEST(tip::iri::grammar::parse::ihost_grammar, Host, tip::iri::host,
	::testing::Values(
		ParseHost::make_test_data("google.com", tip::iri::host{ "google.com" }),
		ParseHost::make_test_data("127.0.0.1", tip::iri::host{ "127.0.0.1" }),
		ParseHost::make_test_data("127.com", tip::iri::host{ "127.com" }),
		ParseHost::make_test_data("[ffaa:1234::aadd:255.255.255.0]",
					tip::iri::host{ "[ffaa:1234::aadd:255.255.255.0]" })
	)
);

GRAMMAR_TEST(tip::iri::grammar::parse::iauthority_grammar, IAuthority,
	::testing::Values(
		"google.com", "www.mail.ru", "[::8]", "127.0.0.1", "[ffaa:1234::aadd:255.255.255.0]",
		"user@google.com", "user:password@www.mail.ru", "user:password@[::8]:8080",
		"127.0.0.1", "user:password@[ffaa:1234::aadd:255.255.255.0]:5432"
	),
	::testing::Values(
		"±", "§", "`", "?asdf", "%zxcv"
	)
);

GRAMMAR_PARSE_TEST(tip::iri::grammar::parse::iauthority_grammar, Authority, tip::iri::authority,
		::testing::Values(
			ParseAuthority::make_test_data("google.com",
					tip::iri::authority{ tip::iri::host{"google.com"} }),
			ParseAuthority::make_test_data("user@google.com",
					tip::iri::authority{
						tip::iri::host{"google.com"},
						tip::iri::port(),
						tip::iri::userinfo{"user"}
					}),
			ParseAuthority::make_test_data("localhost:8080",
					tip::iri::authority{
						tip::iri::host{"localhost"},
						tip::iri::port("8080"),
						tip::iri::userinfo{}
					}),
			ParseAuthority::make_test_data(
					"[ffaa:1234::aadd:255.255.255.0]:8080",
					tip::iri::authority{
						tip::iri::host{"[ffaa:1234::aadd:255.255.255.0]"},
						tip::iri::port{"8080"}
					}),
			ParseAuthority::make_test_data(
					"user:password@[ffaa:1234::aadd:255.255.255.0]:5432",
					tip::iri::authority{
						tip::iri::host{"[ffaa:1234::aadd:255.255.255.0]"},
						tip::iri::port{"5432"},
						tip::iri::userinfo{"user:password"}
					})
		)
);

template < typename InputIterator >
using irelative_part_grammar = tip::iri::grammar::parse::irelative_part_grammar< InputIterator >;

GRAMMAR_TEST(irelative_part_grammar, IRelativePart,
	::testing::Values(
		"//google.com", "//www.mail.ru", "//[::8]", "//127.0.0.1",
		"//[ffaa:1234::aadd:255.255.255.0]",
		"//user@google.com", "//user:password@www.mail.ru/bla/bla/bla",
		"//user:password@[::8]:8080",
		"//127.0.0.1", "//user:password@[ffaa:1234::aadd:255.255.255.0]:5432",
		"bla", "flbla/a:sdf",
		"/", "/bla", "/flbla/asdf"
	),
	::testing::Values(
		"[::8]", "", "[ffaa:1234::aadd:255.255.255.0]", "user:password@[::8]:8080",
		"user:password@[ffaa:1234::aadd:255.255.255.0]:5432"
	)
);

template < typename InputIterator >
using irelative_ref_grammar = tip::iri::grammar::parse::irelative_ref_grammar< InputIterator >;

GRAMMAR_TEST(irelative_ref_grammar, IRelativeRef,
	::testing::Values(
		"//google.com?wtf", "//www.mail.ru", "//[::8]", "//127.0.0.1",
		"//[ffaa:1234::aadd:255.255.255.0]?wtf#there",
		"//user@google.com", "//user:password@www.mail.ru/bla/bla/bla",
		"//user:password@[::8]:8080",
		"//127.0.0.1", "//user:password@[ffaa:1234::aadd:255.255.255.0]:5432",
		"bla", "flbla/a:sdf",
		"/", "/bla", "/flbla/asdf"
	),
	::testing::Values(
		"[::8]", "", "[ffaa:1234::aadd:255.255.255.0]", "user:password@[::8]:8080",
		"user:password@[ffaa:1234::aadd:255.255.255.0]:5432"
	)
);

template < typename InputIterator >
using iri_grammar =
	tip::iri::grammar::parse::iri_grammar< InputIterator, tip::iri::grammar::parse::iquery_grammar< InputIterator > >;

GRAMMAR_TEST(iri_grammar, IRI,
	::testing::Values(
		"http://google.com/webhp?sourceid=chrome-instant&ion=1&espv=2&es_th=1&ie=UTF-8#newwindow=1&q=valid+iri+regex",
		"http://google.com",
		"mailto:user@google.com",
		"https://www.google.com"
	),
	::testing::Values(
		"[::8]", "", "[ffaa:1234::aadd:255.255.255.0]", "user:password@[::8]:8080",
		"user:password@[ffaa:1234::aadd:255.255.255.0]:5432"
	)
);

using iri = tip::iri::basic_iri< tip::iri::query >;

GRAMMAR_PARSE_TEST(iri_grammar, IRI, iri,
	::testing::Values(
		ParseIRI::make_test_data("https://www.google.com",
			iri{
				tip::iri::scheme{"https"},
				tip::iri::authority{ tip::iri::host{"www.google.com"} },
				tip::iri::path{true}
			}),
		ParseIRI::make_test_data("https://www.google.com/foo?bar#baz",
			iri{
				tip::iri::scheme{"https"},
				tip::iri::authority{ tip::iri::host{"www.google.com"}},
				tip::iri::path{true, {"foo"}},
				tip::iri::query{"bar"},
				tip::iri::fragment{"baz"}
			}),
		ParseIRI::make_test_data("ftp://user:password@[1:2::8]:8080/foo/bar?baz",
			iri{
				tip::iri::scheme{"ftp"},
				tip::iri::authority{
					tip::iri::host{"[1:2::8]"},
					tip::iri::port{"8080"},
					tip::iri::userinfo{"user:password"}
				},
				tip::iri::path{true, {"foo", "bar"}},
				tip::iri::query{"baz"}
			})
	)
);
