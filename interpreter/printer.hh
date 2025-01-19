#ifndef PRINTER_H
#define PRINTER_H

#include "ast.hh"
#include "parser_output.hh"

struct Printer : public ParserOutput, StatementVisitor {
    void interpret(std::vector<std::unique_ptr<Statement>> statements) override;

    void visit(const SimpleCommand *com) override;
    void visit(const Assignment *assignment) override;
    void visit(const Pipeline *pipeline) override;
    void visit(const Subshell *s) override;

    void prompt() override {}
};

#endif
