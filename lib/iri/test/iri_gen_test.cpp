/*
 * iri_gen_test.cpp
 *
 *  Created on: Aug 19, 2015
 *      Author: zmij
 */

#include "grammar/grammar_gen_test.hpp"
#include <tip/iri/grammar/iri_generate.hpp>

namespace gen = tip::iri::grammar::gen;

GRAMMAR_GEN_TEST(gen::sub_delims_grammar, SubDelims, char,
	::testing::Values(
		GenerateSubDelims::make_test_data('!', "!"),
		GenerateSubDelims::make_test_data('$', "$")
	)
);

GRAMMAR_GEN_TEST(gen::gen_delims_grammar, GenDelims, char,
	::testing::Values(
		GenerateSubDelims::make_test_data(':', ":"),
		GenerateSubDelims::make_test_data('?', "?")
	)
);

GRAMMAR_GEN_TEST(gen::reserved_grammar, Reserved, char,
	::testing::Values(
		GenerateReserved::make_test_data('!', "!"),
		GenerateReserved::make_test_data('$', "$"),
		GenerateReserved::make_test_data(':', ":"),
		GenerateReserved::make_test_data('?', "?")
	)
);

GRAMMAR_GEN_TEST(gen::unreserved_grammar, Unreserved, std::string,
	::testing::Values(
		GenerateUnreserved::make_test_data("a", "a"),
		GenerateUnreserved::make_test_data("0", "0"),
		GenerateUnreserved::make_test_data("-", "-")
	)
);

GRAMMAR_GEN_TEST(gen::pct_encoded_grammar, PCT, char,
	::testing::Values(
		GeneratePCT::make_test_data( ' ', "%20" ),
		GeneratePCT::make_test_data( '~', "%7e" ),
		GeneratePCT::make_test_data( 9, "%09" )
	)
);

GRAMMAR_GEN_TEST(gen::iunreserved_grammar, IUnreserved, std::string,
	::testing::Values(
		GenerateUnreserved::make_test_data("a", "a"),
		GenerateUnreserved::make_test_data("0", "0"),
		GenerateUnreserved::make_test_data("-", "-"),
		GenerateUnreserved::make_test_data("%xa0", "%xa0")
	)
);

GRAMMAR_GEN_TEST(gen::ipath_grammar, Path, tip::iri::path,
	::testing::Values(
		GeneratePath::make_test_data( {false, { "foo", "bar" }}, "foo/bar" ),
		GeneratePath::make_test_data( {true, { "foo", "bar" }}, "/foo/bar" ),
		GeneratePath::make_test_data( {true}, "/" )

	)
);
