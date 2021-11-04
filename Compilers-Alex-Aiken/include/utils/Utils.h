#pragma once

#ifdef LEXER_STANDALONE
#define LEXER_VERBOSE
#endif // LEXER_STANDALONE

#ifdef LEXER_FULL_VERBOSE
#define LEXER_VERBOSE
#define LEXER_FULL_VERBOSE_ONLY(text) text
#else
#define LEXER_FULL_VERBOSE_ONLY(text)
#endif // LEXER_FULL_VERBOSE

#ifdef PARSER_FULL_VERBOSE
#define PARSER_FULL_VERBOSE_ONLY(text) text
#else
#define PARSER_FULL_VERBOSE_ONLY(text)
#endif // PARSER_FULL_VERBOSE

#ifdef PARSER_STANDALONE
#define PARSER_VERBOSE
#endif // PARSER_STANDALONE

#ifdef SEMANT_STANDALONE
#define SEMANT_VERBOSE
#endif // PARSER_STANDALONE

#ifdef SEMANT_FULL_VERBOSE
#define SEMANT_FULL_VERBOSE_ONLY(text) text
#else
#define SEMANT_FULL_VERBOSE_ONLY(text)
#endif // SEMANT_FULL_VERBOSE

#if defined(LEXER_VERBOSE) || defined(PARSER_VERBOSE)
#include <string>

std::string get_printable_string(const std::string &str);
#endif // LEXER_VERBOSE || PARSER_VERBOSE