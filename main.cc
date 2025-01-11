#include "interpreter/interpreter.hh"
#include "parser.hh"
#include "parser/lexer.hh"

int main()
{
    Lexer lexer;
    Interpreter interpreter;
    yy::parser parser(lexer, interpreter);
    return parser();
}
