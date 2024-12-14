#include "ast.hh"
#include <iostream>

void interpretStatement(Statement *statement)
{
    SimpleCommand *com;
    Pipeline *pipe;
    Assignment *a;
    SubProgram *s;

    if (com = dynamic_cast<SimpleCommand *>(statement))
    {
        std::cout << "SimpleCommand: " << com->name;
        if (!com->args.empty())
            std::cout << " Args:";
        for (auto arg : com->args)
            std::cout << ' ' << arg;
        if (!com->redirections.empty())
            std::cout << " Red:";
        for (auto red : com->redirections)
            std::cout << ' ' << red->file;
        std::cout << '\n';
    }
    else if (pipe = dynamic_cast<Pipeline *>(statement))
    {
        interpretStatement(pipe->lhs);
        interpretStatement(pipe->rhs);
    }
    else if (s = dynamic_cast<SubProgram *>(statement))
    {
        std::cout << "SubProgram: " << s->cmds << "\n";
    }
    else
    {
        std::cerr << "Not recognized statement\n";
    }

    std::cout << '\n';
}

void interpret(std::vector<Statement *> statements)
{
    for (Statement *statement : statements)
    {
        interpretStatement(statement);
        delete statement;
    }
}
