/*
 * grammar_gen_test.hpp
 *
 *  Created on: Aug 19, 2015
 *      Author: zmij
 */

#ifndef HTTP_GRAMMAR_GEN_TEST_HPP_
#define HTTP_GRAMMAR_GEN_TEST_HPP_

#include <gtest/gtest.h>
#include <string>
#include <sstream>
#include <iterator>

template < template<typename> class Grammar, typename Input >
class GrammarGenTest : public ::testing::TestWithParam< std::pair<Input, std::string> > {
protected:
	typedef std::ostream_iterator<char> output_iterator;
	typedef Grammar< output_iterator > grammar_type;
	grammar_type gen;
public:
	typedef Input input_type;
	typedef std::pair< input_type, std::string > param_type;
	static param_type
	make_test_data(input_type const& in, std::string const& expect)
	{
		return std::make_pair(in, expect);
	}
};

#define GRAMMAR_GEN_TEST(grammar_name, test_name, input_type, generator) \
typedef GrammarGenTest<grammar_name, input_type> Generate##test_name; \
TEST_P(Generate##test_name, TestGenerate) \
{ \
	namespace karma = boost::spirit::karma;\
	ParamType param = GetParam(); \
	std::ostringstream os; \
	output_iterator out(os); \
	EXPECT_TRUE(karma::generate(out, gen, param.first)); \
	EXPECT_EQ(param.second, os.str()); \
} \
INSTANTIATE_TEST_CASE_P(GeneratorTest, Generate##test_name, generator)

#endif /* HTTP_GRAMMAR_GEN_TEST_HPP_ */
