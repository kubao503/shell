#ifndef PARSER_OUTPUT_H
#define PARSER_OUTPUT_H

#include <vector>

class Statement;

template <typename T>
class Wrapper;

class ParserOutput
{
public:
    virtual void interpret(std::vector<Wrapper<Statement>> statements) = 0;
};

#endif
