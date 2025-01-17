#include "printer.hh"

#include <iostream>

void Printer::interpret(std::vector<std::unique_ptr<Statement>> statements) {
    for (const auto &statement: statements)
        statement->accept(this);
}

void Printer::visit(const SimpleCommand *com) {
    std::cout << "SimpleCommand: " << com->name;
    if (!com->args.empty())
        std::cout << " Args:";
    for (auto arg: com->args)
        std::cout << ' ' << arg;
    if (!com->redirections.empty())
        std::cout << " Red:";
    for (auto red: com->redirections)
        std::cout << ' ' << red.file;
    std::cout << '\n';
}

void Printer::visit(const Pipeline *pipeline) {
    pipeline->lhs->accept(this);
    pipeline->rhs->accept(this);
}

void Printer::visit(const Subshell *s) {
    std::cout << "SubShell: " << s->cmds << '\n';
}

void Printer::visit(const Assignment *a) {
    std::cout << "Assignment: " << a->var << '=' << a->value << '\n';
}
