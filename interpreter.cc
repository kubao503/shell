#include "interpreter.hh"

#include "ast.hh"
#include <iostream>
#include <sys/wait.h>


void Interpreter::interpretStatement(const Statement *statement, int pipelineSize, int pipelineStep)
{
    const SimpleCommand *com;
    const Pipeline *pipeline;
    const Assignment *a;
    const SubShell *s;

    if (com = dynamic_cast<const SimpleCommand *>(statement))
    {
        // std::cout << "SimpleCommand: " << com->name;
        // if (!com->args.empty())
        //     std::cout << " Args:";
        // for (auto arg : com->args)
        //     std::cout << ' ' << arg;
        // if (!com->redirections.empty())
        //     std::cout << " Red:";
        // for (auto red : com->redirections)
        //     std::cout << ' ' << red.file;
        // std::cout << '\n';

        runSimpleCommand(com);
        std::cout << '\n';

    }
    else if (pipeline = dynamic_cast<const Pipeline *>(statement))
    {
        runPipeline(statement);
        std::cout << "\n";
    }
    else if (s = dynamic_cast<const SubShell *>(statement))
    {
        std::cout << "SubShell: " << s->cmds << "\n";
    }
    else
    {
        std::cerr << "Not recognized statement\n";
    }

}

void Interpreter::interpret(std::vector<std::unique_ptr<Statement>> statements)
{
    for (const auto &statement : statements)
        interpretStatement(statement.get());
}

void Interpreter::createVectorFromPipeline(const Statement* statement, std::vector<const SimpleCommand*> *vectorOfCommands)
{
    const SimpleCommand *com;
    const Pipeline *pipe;
    const Assignment *a;
    const SubShell *s;

    if (com = dynamic_cast<const SimpleCommand *>(statement))
    {
        vectorOfCommands->push_back(com);
    }
    else if (pipe = dynamic_cast<const Pipeline *>(statement))
    {
        createVectorFromPipeline(pipe->lhs.get(), vectorOfCommands);
        createVectorFromPipeline(pipe->rhs.get(), vectorOfCommands);
    }
}

void Interpreter::runSimpleCommand(const SimpleCommand *com)
{
    if (fork() == 0)
    {
        if (com->args.empty())
        {
            execlp(com->name.c_str(), com->name.c_str(), NULL);
        }
        else
        {
            executeCommandArgs(com);
        }
    }
}

void Interpreter::runPipeline(const Statement *statement)
{
        std::vector<const SimpleCommand*> vectorOfCommands;
        createVectorFromPipeline(statement, &vectorOfCommands);
        int fds1[2];
        int fds2[2];
        pipe(fds1);
        pipe(fds2);


        for (size_t i = 0; i < vectorOfCommands.size(); i++)
        {
            if (i == 0 )
            {
                runSimpleCommandStartPipe(vectorOfCommands.at(i), fds1, fds2);
            }
            else if (i == vectorOfCommands.size() - 1 && i % 2 == 0)
            {
                runSimpleCommandEndPipe(vectorOfCommands.at(i), fds1, fds2);
            }
            else if (i == vectorOfCommands.size() - 1 && i % 2 == 1)
            {
                runSimpleCommandEndPipe(vectorOfCommands.at(i), fds2, fds1);
            }
            else if (i % 2 == 1)
            {
                runSimpleCommandPipe(vectorOfCommands.at(i), fds1, fds2);
            }
            else
            {
                runSimpleCommandPipe(vectorOfCommands.at(i), fds2, fds1);
            }
        }

        close(fds1[0]); close(fds1[1]);
        close(fds2[0]); close(fds2[1]);

        for (size_t i = 0; i < vectorOfCommands.size(); i++)
        {
            wait(NULL);
        }
}

void Interpreter::runSimpleCommandStartPipe(const SimpleCommand *com, int fds1[] , int fds2[])
{

    if (fork() == 0)
    {
        close(fds2[0]); close(fds2[1]);
        dup2(fds1[1], 1);
        close(fds1[1]); close(fds1[0]);
        executeCommand(com);
    }
}

void Interpreter::runSimpleCommandEndPipe(const SimpleCommand *com, int fds1[], int fds2[])
{

    if (fork() == 0)
    {
        close(fds1[1]); close(fds1[0]);
        dup2(fds2[0], 0);
        close(fds2[0]); close(fds2[1]);
        executeCommand(com);
    }
}

void Interpreter::runSimpleCommandPipe(const SimpleCommand *com, int fds1[], int fds2[])
{

    if (fork() == 0)
    {
        close(fds1[1]); close(fds2[0]);
        dup2(fds1[0], 0); dup2(fds2[1], 1);
        close(fds1[0]); close(fds2[1]);
        executeCommand(com);
    }
}

void Interpreter::executeCommandArgs(const SimpleCommand *com)
{
    std::vector<const char*> args;
    args.push_back(com->name.c_str());
    for (const auto& arg : com->args)
    {
        args.push_back(arg.c_str());
    }
    args.push_back(NULL);
    execvp(com->name.c_str(), const_cast<char* const*>(args.data()));
}

void Interpreter::executeCommand(const SimpleCommand *com)
{
    if (com->args.empty())
    {
        execlp(com->name.c_str(), com->name.c_str(), NULL);
    }
    else
    {
        executeCommandArgs(com);
    }
}