#include "codegen/mips/Assembler.h"
#include "codegen/mips/CodeGen.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "semant/Semant.h"
#include "utils/Utils.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <vector>

/**
 * @brief Parse
 *
 * @param files Indexes of files in argv
 * @param argv Command line arguments
 * @return Vector of programs ASTs for each file
 */
std::vector<std::shared_ptr<ast::Program>> do_parse(const std::vector<int> &files, char *argv[]);

/**
 * @brief Do semantic analysis
 *
 * @param programs Vector of programs' ASTs for each file
 * @return One AST for all files
 */
std::shared_ptr<semant::ClassNode> do_semant(const std::vector<std::shared_ptr<ast::Program>> &programs);

/**
 * @brief Generate code
 *
 * @param program One AST for all files
 * @return Buffer with instructions
 */
codegen::CodeBuffer do_codegen(const std::shared_ptr<semant::ClassNode> &program);

int main(int argc, char *argv[])
{
    std::vector<int> files;

#ifdef DEBUG
    files = process_args(argv, argc);
#else
    files.resize(argc - 1);
    std::iota(files.begin(), files.end(), 1);
#endif // DEBUG

    const auto parsed_program = do_parse(files, argv);
    const auto analysed_program = do_semant(parsed_program);
    const auto buffer = do_codegen(analysed_program);

    std::string src_name(argv[files[0]]);
    std::ofstream out_file(std::string(src_name.substr(0, src_name.find_last_of(".")) + ".s"));

    out_file << static_cast<std::string>(buffer);

    return 0;
}

std::vector<std::shared_ptr<ast::Program>> do_parse(const std::vector<int> &files, char *argv[])
{
    std::vector<std::shared_ptr<ast::Program>> programs;

    for (const auto &i : files)
    {
        DEBUG_ONLY(if (TokensOnly) {
            lexer::Lexer l(argv[i]);
            std::cout << "#name \"" << l.file_name() << "\"" << std::endl;

            auto token = l.next();
            while (token.has_value())
            {
                token = l.next();
            }

            continue;
        });

        parser::Parser parser(std::make_shared<lexer::Lexer>(argv[i]));
        if (const auto ast = parser.parse_program())
        {
            programs.push_back(ast);
        }
        else
        {
            std::cout << parser.error_msg() << std::endl;
            exit(-1);
        }
    }

    DEBUG_ONLY(if (TokensOnly) { exit(0); });

    return programs;
}

std::shared_ptr<semant::ClassNode> do_semant(const std::vector<std::shared_ptr<ast::Program>> &programs)
{
    semant::Semant semant(std::move(programs));
    const auto result = semant.infer_types_and_check();

    if (!result.second)
    {
        std::cout << semant.error_msg() << std::endl;
        exit(-1);
    }

    DEBUG_ONLY(if (PrintFinalAST) { ast::dump_program(*(result.second)); });

    return result.first;
}

codegen::CodeBuffer do_codegen(const std::shared_ptr<semant::ClassNode> &program)
{
    codegen::CodeGen codegen(program);
    return codegen.emit();
}