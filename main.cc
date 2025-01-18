#include "interpreter/interpreter.hh"
#include "parser.hh"
#include "parser/lexer.hh"
#include <sstream>

void prompt();

int main(int argc, char* argv[]) {
    if(argc > 1 && std::string(argv[1]) == "-c") {
        std::string input;
        for(int i = 2; i < argc; i++) {
            input += argv[i];
            input += " ";
        }
        input.resize(input.size() - 1);
        std::istringstream inputStream(input);
        Lexer lexer(&inputStream);
        Interpreter interpreter;
        yy::parser parser(lexer, interpreter, false);
        return parser();
    }
    Lexer lexer;
    Interpreter interpreter;
    yy::parser parser(lexer, interpreter, true);
    interpreter.prompt();
    return parser();
}
