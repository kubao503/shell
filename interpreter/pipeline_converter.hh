//
// Created by jani on 1/26/25.
//

#ifndef UXSH_PIPELINE_CONVERTER_HH
#define UXSH_PIPELINE_CONVERTER_HH


#include "ast.hh"
#include <vector>

class PipelineConverter: public StatementVisitor{
private:
    std::vector<const Statement *> result;

public:
    auto getResult() { return result; }
    void visit(const Subshell *subshell) override;
    void visit(const Assignment *assignment) override;
    void visit(const SimpleCommand *simpleCommand) override;
    void visit(const Pipeline *pipeline) override;
};


#endif//UXSH_PIPELINE_CONVERTER_HH
