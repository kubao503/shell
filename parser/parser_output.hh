#ifndef PARSER_OUTPUT_H
#define PARSER_OUTPUT_H

#include <vector>

class Statement;

class ParserOutput
{
public:
    virtual void interpret(std::vector<Statement *> statements) = 0;
};

#endif
