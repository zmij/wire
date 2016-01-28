/**
 * grammar_test.hpp
 *
 *  Created on: 12 авг. 2015 г.
 *      Author: zmij
 */

#ifndef HTTP_GRAMMAR_PARSE_TEST_HPP_
#define HTTP_GRAMMAR_PARSE_TEST_HPP_

#include <gtest/gtest.h>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>

template < template<typename> class Grammar >
class GrammarTest : public ::testing::TestWithParam< std::string > {
protected:
	typedef std::string::const_iterator string_iterator;
	typedef Grammar< string_iterator > grammar_type;

	typedef std::istreambuf_iterator<char> istreambuf_iterator;
	typedef boost::spirit::multi_pass< istreambuf_iterator > stream_iterator;
	typedef Grammar< stream_iterator > stream_grammar_type;
	grammar_type			parser;
	stream_grammar_type		stream_parser;
};

template< template<typename> class Grammar, typename Result >
class GrammarParseTest : public ::testing::TestWithParam< std::pair< std::string, Result >> {
protected:
	typedef std::string::const_iterator string_iterator;
	typedef Grammar< string_iterator > grammar_type;

	typedef std::istreambuf_iterator<char> istreambuf_iterator;
	typedef boost::spirit::multi_pass< istreambuf_iterator > stream_iterator;
	typedef Grammar< stream_iterator > stream_grammar_type;
	grammar_type			parser;
	stream_grammar_type		stream_parser;
public:
	typedef Result result_type;
	typedef std::pair< std::string, result_type > param_type;
	static param_type
	make_test_data(std::string const& input, result_type const& expect)
	{
		return std::make_pair(input, expect);
	}
};

#define GRAMMAR_TEST(grammar_name, test_name, valid_generator, invalid_generator) \
typedef GrammarTest< grammar_name > Valid##test_name; \
typedef GrammarTest< grammar_name > Invalid##test_name; \
TEST_P(Valid##test_name, ValidInput) \
{ \
	namespace qi = boost::spirit::qi; \
	std::string input = GetParam(); \
	string_iterator first = input.begin(); \
	string_iterator last = input.end(); \
	EXPECT_TRUE(qi::parse(first, last, parser)); \
	EXPECT_EQ(last, first); \
} \
TEST_P(Valid##test_name, ValidStreamInput) \
{ \
	namespace qi = boost::spirit::qi; \
	std::string input = GetParam(); \
	std::istringstream is(input); \
	stream_iterator in = stream_iterator(istreambuf_iterator(is)); \
	stream_iterator eos = stream_iterator(istreambuf_iterator()); \
	EXPECT_TRUE(qi::parse(in, eos, stream_parser)); \
} \
INSTANTIATE_TEST_CASE_P(ParserTest, Valid##test_name, valid_generator); \
TEST_P(Invalid##test_name, InvalidInput) \
{ \
	namespace qi = boost::spirit::qi; \
	std::string input = GetParam(); \
	std::string::const_iterator first = input.begin(); \
	std::string::const_iterator last = input.end(); \
	EXPECT_FALSE(qi::parse(first, last, parser) && first == last); \
} \
INSTANTIATE_TEST_CASE_P(ParserTest, Invalid##test_name, invalid_generator)

#define GRAMMAR_PARSE_TEST(grammar_name, test_name, result_type, generator) \
typedef GrammarParseTest<grammar_name, result_type> Parse##test_name; \
TEST_P(Parse##test_name, TestParse) \
{ \
	namespace qi = boost::spirit::qi;\
	ParamType param = GetParam(); \
	result_type res; \
	string_iterator f = param.first.begin(); \
	string_iterator l = param.first.end(); \
	EXPECT_TRUE(qi::parse(f, l, parser, res)); \
	EXPECT_EQ(l, f); \
	EXPECT_EQ(param.second, res); \
} \
TEST_P(Parse##test_name, TestStreamParse) \
{ \
	namespace qi = boost::spirit::qi; \
	ParamType param = GetParam(); \
	result_type res; \
	std::istringstream is(param.first); \
	stream_iterator in = stream_iterator(istreambuf_iterator(is)); \
	stream_iterator eos = stream_iterator(istreambuf_iterator()); \
	EXPECT_TRUE(qi::parse(in, eos, stream_parser, res)); \
	EXPECT_EQ(param.second, res); \
} \
INSTANTIATE_TEST_CASE_P(ParserTest, Parse##test_name, generator)

/*
TEST_P(Valid##test_name, ValidStreambufInput) \
{ \
	namespace qi = boost::spirit::qi; \
	namespace spirit = boost::spirit; \
	std::string input = GetParam(); \
	std::istringstream is(input); \
	streambuf_multipass_iterator f = \
		spirit::make_default_multi_pass(streambuf_base_iterator(is)) \
	streambuf_multipass_iterator l = \
		spirit::make_default_multi_pass(streambuf_base_iterator()) \
	EXPECT_TRUE(qi::parse(f, l, streambuf_parser)); \
} \
 */

#endif /* HTTP_GRAMMAR_PARSE_TEST_HPP_ */
