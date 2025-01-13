%{
    #include "lexer.hh"
    #include "parser.hh"
%}

%option c++ noyywrap

%option yyclass="Lexer"

id [^[:space:]]+

%%

\(.*\)              { return yy::parser::make_SUBSHELL(yytext); }
\n                  { return '\n'; }
\|                  { return '|'; }
;                   { return ';'; }
\<                  { return '<'; }
\>                  { return '>'; }
{id}=[[:alnum:]]*   { return yy::parser::make_ASSIGNMENT_WORD(yytext); }
{id}                { return yy::parser::make_WORD(yytext); }
.                   ;
