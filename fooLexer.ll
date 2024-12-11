%{
    #include "FooLexer.hh"
    #include "fooParser.tab.hh"
%}

%option c++ noyywrap

%option yyclass="FooLexer"

letter [[:alpha:]]

%%

{letter}+={letter}* { return yy::parser::make_ASSIGNMENT_WORD(yytext); }
{letter}+           { return yy::parser::make_WORD(yytext); }
\n                  { return yy::parser::make_NEWLINE(); }
.                   ;
