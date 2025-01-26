//
// Created by jani on 1/26/25.
//

#include "pipeline_converter.hh"

void PipelineConverter::visit(const SimpleCommand *simpleCommand) {
    result.push_back(simpleCommand);
}

void PipelineConverter::visit(const Pipeline *pipeline) {
    pipeline->lhs->accept(this);
    pipeline->rhs->accept(this);
}

void PipelineConverter::visit(const Subshell *subshell) {
    result.push_back(subshell);
}

void PipelineConverter::visit(const Assignment *assignment) {
    result.push_back(assignment);
}