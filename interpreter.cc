#include "interpreter.hh"

#include "ast.hh"
#include <iostream>

void Interpreter::interpretStatement(const Statement *statement)
{
    const SimpleCommand *com;
    const Pipeline *pipe;
    const Assignment *a;
    const SubShell *s;

    if (com = dynamic_cast<const SimpleCommand *>(statement))
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
    else if (pipe = dynamic_cast<const Pipeline *>(statement))
    {
        interpretStatement(pipe->lhs.get());
        interpretStatement(pipe->rhs.get());
    }
    else if (s = dynamic_cast<const SubShell *>(statement))
    {
        std::cout << "SubShell: " << s->cmds << "\n";
    }
    else
    {
        std::cerr << "Not recognized statement\n";
    }

    std::cout << '\n';
}

void Interpreter::interpret(std::vector<std::unique_ptr<Statement>> statements)
{
    for (const auto &statement : statements)
        interpretStatement(statement.get());
}
