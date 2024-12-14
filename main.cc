#include "parser/lexer.hh"
#include "parser.hh"
#include "interpreter.hh"

int main()
{
    Lexer lexer;
    Interpreter interpreter;
    yy::parser parser(lexer, interpreter);
    return parser();
}
