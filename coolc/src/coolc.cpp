#include "semant/Semant.h"
#include "parser/Parser.h"
#include "lexer/Lexer.h"
#include "codegen/mips/CodeGen.h"

#ifdef LEXER_STANDALONE
int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        lexer::Lexer l(argv[i]);
        l.display();

        auto token = l.next();
        while (token.has_value())
        {
            token->display();
            token = l.next();
        }
    }

    return 0;
}
#endif //LEXER_STANDALONE

#ifdef PARSER_STANDALONE
int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        parser::Parser parser(std::make_shared<lexer::Lexer>(argv[i]));
        if (const auto ast = parser.parse_program())
        {
            ast::dump_program(*ast);
        }
        else
        {
            std::cout << parser.get_error_msg() << std::endl;
            return -1;
        }
    }

    return 0;
}
#endif //PARSER_STANDALONE

#ifdef SEMANT_STANDALONE
int main(int argc, char *argv[])
{
    std::vector<std::shared_ptr<ast::Program>> programs;
    for (int i = 1; i < argc; i++)
    {
        parser::Parser parser(std::make_shared<lexer::Lexer>(argv[i]));
        if (const auto ast = parser.parse_program())
        {
            programs.push_back(ast);
        }
        else
        {
            std::cout << parser.get_error_msg() << std::endl;
            return -1;
        }
    }

    semant::Semant semant(std::move(programs));
    const auto result = semant.infer_types_and_check();
    if (result.second)
    {
        ast::dump_program(*(result.second));
    }
    else
    {
        std::cout << semant.get_error_msg() << std::endl;
        return -1;
    }

    return 0;
}
#endif // SEMANT_STANDALONE

#ifdef COOLC
#include <fstream>

int main(int argc, char *argv[])
{
    // lexer/parser stage
    std::vector<std::shared_ptr<ast::Program>> programs;
    for (int i = 1; i < argc; i++)
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
#endif // COOLC