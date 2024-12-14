#include "parser/lexer.hh"
#include "parser.hh"

int main()
{
    Lexer lexer;
    yy::parser parser(lexer);
    return parser();
}
