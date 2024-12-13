%{
    #include "FooLexer.hh"
    #include "fooParser.tab.hh"
%}

%option c++ noyywrap

%option yyclass="FooLexer"

id [[:alnum:]]*

%%

{id}=[[:alnum:]]*   { return yy::parser::make_ASSIGNMENT_WORD(yytext); }
{id}                { return yy::parser::make_WORD(yytext); }
\(.*\)              { return yy::parser::make_SUBSHELL(yytext); }
\n                  { return '\n'; }
\|                  { return '|'; }
;                   { return ';'; }
\<                  { return '<'; }
\>                  { return '>'; }
.                   ;
