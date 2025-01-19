%{
    #include "lexer.hh"
    #include "parser.hh"

    std::string unwrap(const char* text, size_t len);

    #define DEBUG(text) (std::cout << text << ": " << yytext << "\n")
%}

%option c++ noyywrap

%option yyclass="Lexer"

id      [^[:space:]()'\|;<>]
expr    ({id}|'[^']*')

%%

\([^()]*\)          { return yy::parser::make_SUBSHELL(unwrap(yytext, yyleng)); }
\n                  { return '\n'; }
\|                  { return '|'; }
;                   { return ';'; }
\<                  { return '<'; }
\>                  { return '>'; }
{id}+={expr}*       { return yy::parser::make_ASSIGNMENT_WORD(yytext); }
{expr}+             { return yy::parser::make_WORD(yytext); }
.                   ;

%%

std::string unwrap(const char* text, size_t len) {
    return std::string(text + 1, text + len - 1);
}
