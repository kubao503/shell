#include "parser/FooLexer.hh"
#include "parser/parser.hh"

int main()
{
    FooLexer lexer;
    yy::parser parser(lexer);
    return parser();
}
