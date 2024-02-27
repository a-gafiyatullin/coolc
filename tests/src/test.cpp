#include "test.h"
#include <filesystem>
#include <fstream>
#include <iostream>

std::string CompilerParameterizedTest::ProjHomeDir;
std::string CompilerParameterizedTest::CompilerTestsDir;
std::string CompilerParameterizedTest::CompilerTestsResultsDir;
std::string CompilerParameterizedTest::CompilerTestsEtalonsDir;

std::vector<TestParam> CompilerParameterizedTest::read_test_cases_from_disk(const std::string &tests_type)
{
    auto proj_home_dir_obj = std::filesystem::current_path();
    ProjHomeDir = proj_home_dir_obj;
    // try to find project root
    while (!std::filesystem::exists(ProjHomeDir + "/bin"))
    {
        proj_home_dir_obj = proj_home_dir_obj.parent_path();
        ProjHomeDir = proj_home_dir_obj;
    }
    CompilerTestsDir = ProjHomeDir + "/tests/" + tests_type;
    CompilerTestsResultsDir = CompilerTestsDir + "/results/";
    CompilerTestsEtalonsDir = CompilerTestsDir + "/etalons/";

    std::cout << "PROJECT HOME DIR: " << ProjHomeDir << std::endl;
    std::cout << "TESTS DIR: " << CompilerTestsDir << std::endl;

    std::vector<TestParam> tests;

    for (const auto &file : std::filesystem::directory_iterator(CompilerTestsResultsDir))
    {
        const std::string result = file.path();
        std::string etalon = result;
        etalon.replace(etalon.find(CompilerTestsResultsDir), CompilerTestsResultsDir.length(), CompilerTestsEtalonsDir);
        tests.push_back({result, etalon});
    }

    return tests;
}
