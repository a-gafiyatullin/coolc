#ifdef LEXER_STANDALONE
#define LEXER_VERBOSE

#endif // LEXER_STANDALONE

#ifdef LEXER_FULL_VERBOSE
#define LEXER_VERBOSE

#define log_match(type, str, pos) std::cout << "Matched " << type << " " << str << " in position " << pos << std::endl;
#define log(str) std::cout << str << std::endl;

#else

#define log_match(type, str, pos)
#define log(str)

#endif // LEXER_FULL_VERBOSE
