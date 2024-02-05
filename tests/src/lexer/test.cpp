#include "test.h"

class LexerParameterizedTest : public CompilerParameterizedTest
{
  public:
    static std::vector<TestParam> read_lexer_tests_from_disk() { return read_test_cases_from_disk("lexer/"); }
};

TEST_P(LexerParameterizedTest, LexAsExpected)
{
    std::ifstream result(GetParam().first);
    const std::string result_content((std::istreambuf_iterator<char>(result)), std::istreambuf_iterator<char>());

    std::ifstream etalon(GetParam().second);
    const std::string etalon_content((std::istreambuf_iterator<char>(etalon)), std::istreambuf_iterator<char>());

    ASSERT_EQ(result_content, etalon_content);
}

INSTANTIATE_TEST_SUITE_P(LexerTests, LexerParameterizedTest,
                         testing::ValuesIn(LexerParameterizedTest::read_lexer_tests_from_disk()));