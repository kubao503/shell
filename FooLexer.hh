#ifndef LEXER_H
#define LEXER_H

#undef YY_DECL
#define YY_DECL yy::parser::symbol_type FooLexer::get_next_token()

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "fooParser.tab.hh"
#include <string>

class FooLexer : public yyFlexLexer
{
public:
    virtual yy::parser::symbol_type get_next_token();
};

#endif
