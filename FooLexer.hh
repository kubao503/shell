#ifndef LEXER_H
#define LEXER_H

#undef YY_DECL
#define YY_DECL int FooLexer::yylex(std::string *const yylval)

#include <string>
#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

class FooLexer : public yyFlexLexer
{
public:
    int yylex(std::string *const yylval);
};

#endif
