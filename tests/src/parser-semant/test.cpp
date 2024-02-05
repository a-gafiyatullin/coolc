#include "test.h"

class ParserSemantParameterizedTest : public CompilerParameterizedTest
{
  public:
    static std::vector<TestParam> read_parser_tests_from_disk() { return read_test_cases_from_disk("parser-semant/"); }
};

TEST_P(ParserSemantParameterizedTest, ParserSemantAsExpected)
{
    std::ifstream result(GetParam().first);
    const std::string result_content((std::istreambuf_iterator<char>(result)), std::istreambuf_iterator<char>());

    std::ifstream etalon(GetParam().second);
    const std::string etalon_content((std::istreambuf_iterator<char>(etalon)), std::istreambuf_iterator<char>());

    ASSERT_TRUE(!result_content.empty());
    ASSERT_TRUE(etalon_content.find(result_content) != -1);
}

INSTANTIATE_TEST_SUITE_P(ParserTests, ParserSemantParameterizedTest,
                         testing::ValuesIn(ParserSemantParameterizedTest::read_parser_tests_from_disk()));