#include "interpreter.hh"

#include "ast.hh"
#include <iostream>
#include <sys/wait.h>

void Interpreter::interpret(std::vector<std::unique_ptr<Statement>> statements)
{
    for (const auto &statement : statements)
        statement->accept(this);
}

void Interpreter::createVectorFromPipeline(const Statement *statement, std::vector<const SimpleCommand *> *commands)
{
    const SimpleCommand *com;
    const Pipeline *pipe;
    const Assignment *a;
    const Subshell *s;

    if (com = dynamic_cast<const SimpleCommand *>(statement))
    {
        commands->push_back(com);
    }
    else if (pipe = dynamic_cast<const Pipeline *>(statement))
    {
        createVectorFromPipeline(pipe->lhs.get(), commands);
        createVectorFromPipeline(pipe->rhs.get(), commands);
    }
}

void Interpreter::visit(const SimpleCommand *simpleCommand) {
    if (fork() == 0)
    {
        if (simpleCommand->args.empty())
        {
            execlp(simpleCommand->name.c_str(), simpleCommand->name.c_str(), NULL);
        }
        else
        {
            executeCommandArgs(simpleCommand);
        }
    }
}

void Interpreter::visit(const Pipeline *pipeline) {
    std::vector<const SimpleCommand *> vectorOfCommands;
    createVectorFromPipeline(pipeline, &vectorOfCommands);

    int fds1[2];

    for (size_t i = 0; i < vectorOfCommands.size(); i++)
    {
        if (i == 0)
        {
            pipe(fds1);
            runSimpleCommandStartPipe(vectorOfCommands.at(i), fds1);
        }
        else if (i == vectorOfCommands.size() - 1)
        {
            runSimpleCommandEndPipe(vectorOfCommands.at(i), fds1);
            close(fds1[0]);
            close(fds1[1]);
        }
        else
        {
            int fds2[2];

            pipe(fds2);
            runSimpleCommandPipe(vectorOfCommands.at(i), fds1, fds2);
            close(fds1[0]);
            close(fds1[1]);
            fds1[0] = fds2[0];
            fds1[1] = fds2[1];
        }
    }

    for (size_t i = 0; i < vectorOfCommands.size(); i++)
    {
        wait(nullptr);
    }
}

void Interpreter::runSimpleCommandStartPipe(const SimpleCommand *com, int fds1[])
{
    if (fork() == 0)
    {
        dup2(fds1[1], 1);
        close(fds1[1]);
        close(fds1[0]);
        executeCommand(com);
    }
}

void Interpreter::runSimpleCommandEndPipe(const SimpleCommand *com, int fds1[])
{
    if (fork() == 0)
    {
        dup2(fds1[0], 0);
        close(fds1[0]);
        close(fds1[1]);
        executeCommand(com);
    }
}

void Interpreter::runSimpleCommandPipe(const SimpleCommand *com, int fds1[], int fds2[])
{
    if (fork() == 0)
    {
        close(fds1[1]);
        close(fds2[0]);
        dup2(fds1[0], 0);
        dup2(fds2[1], 1);
        close(fds1[0]);
        close(fds2[1]);
        executeCommand(com);
    }
}

void Interpreter::executeCommandArgs(const SimpleCommand *com)
{
    std::vector<const char *> args;
    args.push_back(com->name.c_str());
    for (const auto &arg : com->args)
    {
        args.push_back(arg.c_str());
    }
    args.push_back(nullptr);
    execvp(com->name.c_str(), const_cast<char *const *>(args.data()));
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
void Interpreter::visit(const Assignment *assignment) {
}
void Interpreter::visit(const Subshell *subshell) {
}
