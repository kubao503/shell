#ifndef LEXER_H
#define LEXER_H

#undef YY_DECL
#define YY_DECL yy::parser::symbol_type Lexer::get_next_token()

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "parser.hh"
#include <string>

class Lexer : public yyFlexLexer
{
public:
    virtual yy::parser::symbol_type get_next_token();
};

#endif
