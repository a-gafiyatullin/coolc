#include "codegen/mips/CodeGen.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "semant/Semant.h"
#include "utils/Utils.h"
#include <fstream>

int main(int argc, char *argv[])
{
    int files_start_idx = 1;
    DEBUG_ONLY(files_start_idx = process_args(argv + 1, argc - 1));

    // lexer/parser stage
    std::vector<std::shared_ptr<ast::Program>> programs;
    for (int i = files_start_idx; i < argc; i++)
    {
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