#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser_output.hh"

class Interpreter : public ParserOutput
{
    void interpretStatement(Statement *statement);
    void interpret(std::vector<Statement *> statements);
};

#endif
