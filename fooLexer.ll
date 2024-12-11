%{
    #include "FooLexer.hh"
    #include "fooParser.tab.hh"
%}

%option c++ noyywrap

%option yyclass="FooLexer"

letter [[:alnum:]]
alnum [[:alnum:]]

%%

{letter}{alnum}*={alnum}* { return yy::parser::make_ASSIGNMENT_WORD(yytext); }
{letter}{alnum}*           { return yy::parser::make_WORD(yytext); }
\n                  { return yy::parser::make_NEWLINE(); }
\|                  { return yy::parser::make_PIPE(); }
.                   ;
