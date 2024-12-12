%{
    #include "FooLexer.hh"
    #include "fooParser.tab.hh"
%}

%option c++ noyywrap

%option yyclass="FooLexer"

id [[:alpha:]][[:alnum:]]*

%%

{id}=[[:alnum:]]*   { return yy::parser::make_ASSIGNMENT_WORD(yytext); }
{id}                { return yy::parser::make_WORD(yytext); }
\n                  { return yy::parser::make_NEWLINE(); }
\|                  { return yy::parser::make_PIPE(); }
;                   { return yy::parser::make_COMA(); }
.                   ;
