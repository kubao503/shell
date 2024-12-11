#include "FooLexer.hh"
#include "fooParser.tab.hh"

int main()
{
    FooLexer lexer;
    yy::parser parser(lexer);
    return parser();
}
