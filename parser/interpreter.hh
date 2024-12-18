#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser_output.hh"

class Interpreter : public ParserOutput
{
    void interpretStatement(const Wrapper<Statement> &statement);
    void interpret(std::vector<Wrapper<Statement>> statements);
};

#endif
