#include "coolc.h"
#include "parser/Parser.h"
#include <iostream>
#include <numeric>

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
 * @param out_file Result file
 */
void do_codegen(const std::shared_ptr<semant::ClassNode> &program, const std::string &out_file);

int main(int argc, char *argv[])
{
    const auto files = process_args(argv, argc);

    const auto parsed_program = do_parse(files.first, argv);
    const auto analysed_program = do_semant(parsed_program);

    do_codegen(analysed_program, files.second);

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

void do_codegen(const std::shared_ptr<semant::ClassNode> &program, const std::string &out_file)
{
    CODEGEN codegen(program);
    codegen.emit(out_file);
}
