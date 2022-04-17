#include "test.h"

class CodegenParameterizedTest : public CompilerParameterizedTest
{
  public:
    static std::vector<TestParam> read_codegen_tests_from_disk()
    {
        return read_test_cases_from_disk("codegen/");
    }
};

TEST_P(CodegenParameterizedTest, CodegenAsExpected)
{
    std::ifstream result(GetParam().first);
    std::string result_content((std::istreambuf_iterator<char>(result)), std::istreambuf_iterator<char>());

    std::ifstream etalon(GetParam().second);
    std::string etalon_content((std::istreambuf_iterator<char>(etalon)), std::istreambuf_iterator<char>());

    ASSERT_TRUE(result_content.find(etalon_content) != -1);
}

INSTANTIATE_TEST_SUITE_P(CodegenTests, CodegenParameterizedTest,
                         testing::ValuesIn(CodegenParameterizedTest::read_codegen_tests_from_disk()));