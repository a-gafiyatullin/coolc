#include "semant/Semant.h"
#include "parser/Parser.h"
#include "lexer/Lexer.h"

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
        if (auto ast = parser.parse_program())
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

    semant::Semant semant(std::move(programs));
    auto result = semant.infer_types_and_check();
    if (result._program)
    {
        ast::dump_program(*(result._program));
    }
    else
    {
        std::cout << semant.get_error_msg() << std::endl;
        return -1;
    }

    return 0;
}
#endif // SEMANT_STANDALONE