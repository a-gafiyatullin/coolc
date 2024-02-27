#include <gtest/gtest.h>

typedef std::pair<std::string, std::string> TestParam;

class CompilerParameterizedTest : public testing::TestWithParam<TestParam>
{
  private:
    static std::string ProjHomeDir;
    static std::string CompilerTestsDir;
    static std::string CompilerTestsResultsDir;
    static std::string CompilerTestsEtalonsDir;

  protected:
    static std::vector<TestParam> read_test_cases_from_disk(const std::string &tests_type);
};
