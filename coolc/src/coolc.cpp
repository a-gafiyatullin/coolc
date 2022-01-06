#include "codegen/mips/CodeGen.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "semant/Semant.h"
#include "utils/Utils.h"
#include <algorithm>
#include <fstream>
#include <vector>

int main(int argc, char *argv[])
{
    std::vector<int> files;
#ifdef DEBUG
    files = process_args(argv, argc);
#else
    for (int i = 1; i < argc; i++)
    {
        files.push_back(i);
    }
#endif // DEBUG

    // lexer/parser stage
    std::vector<std::shared_ptr<ast::Program>> programs;
    for (const auto &i : files)
    {
        DEBUG_ONLY(if (TokensOnly) {
            lexer::Lexer l(argv[i]);
            std::cout << "#name \"" << l.get_file_name() << "\"" << std::endl;

            auto token = l.next();
            while (token.has_value())
            {
                token = l.next();
            }

            continue;
        });

        parser::Parser parser(std::make_shared<lexer::Lexer>(argv[i]));
        if (auto ast = parser.parse_program())
        {
            programs.push_back(ast);
        }
        else
        {
            std::cout << parser.get_error_msg() << std::endl;
            return -1;
        }
    }

    DEBUG_ONLY(if (TokensOnly) { return 0; });

    // semant stage
    semant::Semant semant(std::move(programs));
    const auto result = semant.infer_types_and_check();

#ifdef DEBUG
    if (PrintFinalAST && result.second)
    {
        ast::dump_program(*(result.second));
    }
#endif // DEBUG

    if (!result.second)
    {
        std::cout << semant.get_error_msg() << std::endl;
        return -1;
    }

    // codegen stage
    codegen::CodeGen codegen(result.first);

    // name of the result code is the name of the first file, but with .s extension
    std::string src_name(argv[1]);
    src_name = src_name.substr(0, src_name.find_last_of("."));
    std::ofstream out_file(src_name + ".s");

    // emit code
    out_file << static_cast<std::string>(codegen.emit());

    return 0;
}