#ifndef PARSER_OUTPUT_H
#define PARSER_OUTPUT_H

#include <vector>
#include <memory>

class Statement;

class ParserOutput
{
public:
    virtual void interpret(std::vector<std::unique_ptr<Statement>> statements) = 0;
    virtual void prompt() = 0;
};

#endif
