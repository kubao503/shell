%{
    #include "lexer.hh"
    #include "parser.hh"

    std::string unwrap(const char* text, size_t len);
%}

%option c++ noyywrap

%option yyclass="Lexer"

id [^[:space:]()'|;<>]+

%%

\([^()]*\)          { return yy::parser::make_SUBSHELL(unwrap(yytext, yyleng)); }
\'[^']*\'           { return yy::parser::make_WORD(unwrap(yytext, yyleng)); }
\n                  { return '\n'; }
\|                  { return '|'; }
;                   { return ';'; }
\<                  { return '<'; }
\>                  { return '>'; }
{id}=[[:alnum:]]*   { return yy::parser::make_ASSIGNMENT_WORD(yytext); }
{id}                { return yy::parser::make_WORD(yytext); }
.                   ;

%%

std::string unwrap(const char* text, size_t len) {
    return std::string(text + 1, text + len - 1);
}