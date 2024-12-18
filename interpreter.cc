#include "interpreter.hh"

#include "ast.hh"
#include "wrapper.hh"
#include <iostream>

void Interpreter::interpretStatement(const Wrapper<Statement> &statement)
{
    const SimpleCommand *com;
    const Pipeline *pipe;
    const Assignment *a;
    const SubShell *s;

    if (com = dynamic_cast<const SimpleCommand *>(statement.get()))
    {
        std::cout << "SimpleCommand: " << com->name;
        if (!com->args.empty())
            std::cout << " Args:";
        for (auto arg : com->args)
            std::cout << ' ' << arg;
        if (!com->redirections.empty())
            std::cout << " Red:";
        for (auto red : com->redirections)
            std::cout << ' ' << red.file;
        std::cout << '\n';
    }
    else if (pipe = dynamic_cast<const Pipeline *>(statement.get()))
    {
        interpretStatement(pipe->lhs);
        interpretStatement(pipe->rhs);
    }
    else if (s = dynamic_cast<const SubShell *>(statement.get()))
    {
        std::cout << "SubShell: " << s->cmds << "\n";
    }
    else
    {
        std::cerr << "Not recognized statement\n";
    }

    std::cout << '\n';
}

void Interpreter::interpret(std::vector<Wrapper<Statement>> statements)
{
    for (const auto &statement : statements)
        interpretStatement(statement);
}
