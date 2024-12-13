#include "structs.hh"
#include <iostream>

void interpret(std::vector<Command *> commands)
{
    Pipeline *pipe;
    Assignment *a;
    SubProgram *s;

    for (Command *command : commands)
    {
        if (pipe = dynamic_cast<Pipeline *>(command))
        {
            for (auto p : pipe->programs)
            {
                std::cout << "Program: " << p->name;
                if (!p->args.empty())
                    std::cout << " Args:";
                for (auto arg : p->args)
                    std::cout << ' ' << arg;
                if (!p->redirections.empty())
                    std::cout << " Red:";
                for (auto red : p->redirections)
                    std::cout << ' ' << red->file;
                std::cout << '\n';
            }
        }
        else if (a = dynamic_cast<Assignment *>(command))
        {
            std::cout << "Assignment: " << a->var << '\n';
        }
        else if (s = dynamic_cast<SubProgram *>(command))
        {
            std::cout << "SubProgram\n";
        }
        else
        {
            std::cerr << "Not recognized command\n";
        }

        std::cout << '\n';
    }
}
