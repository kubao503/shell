%{
    #include "FooLexer.hh"
    #include "fooParser.tab.hh"
%}

%option c++ noyywrap

%option yyclass="FooLexer"

letter [[:alpha:]]

%%

{letter}+={letter}* {
                        *yylval = std::string(yytext, yytext + yyleng);
                        return yy::parser::token::ASSIGNMENT_WORD;
                    }
{letter}+           {
                        *yylval = std::string(yytext, yytext + yyleng);
                        return yy::parser::token::WORD;
                    }
\n                  { return yy::parser::token::NEWLINE; }
.                   ;
