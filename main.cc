#include "interpreter/interpreter.hh"
#include "parser.hh"
#include "parser/lexer.hh"

void prompt();

int main() {
    Lexer lexer;
    Interpreter interpreter;
    yy::parser parser(lexer, interpreter, true);
    prompt();
    return parser();
}
