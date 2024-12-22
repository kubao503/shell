#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser_output.hh"

class Interpreter : public ParserOutput
{
    void interpretStatement(const Statement *statement);
    void interpret(std::vector<std::unique_ptr<Statement>> statements);
};

#endif
