#include "interpreter.hh"

#include "ast.hh"
#include <iostream>
#include <sys/wait.h>
#include <fcntl.h>

void Interpreter::interpret(std::vector<std::unique_ptr<Statement>> statements)
{
    for (const auto &statement : statements)
        statement->accept(this);
}

void Interpreter::visit(const SimpleCommand *simpleCommand) {

    if (fork() == 0)
    {
        if (checkForRedirections(simpleCommand) == 0)
        {
            executeCommand(simpleCommand);
        }
    }
}

void Interpreter::visit(const Pipeline *pipeline) {
    std::vector<const SimpleCommand *> commands = convertPipeline(pipeline);

    int fds1[2];

    for (size_t i = 0; i < commands.size(); i++)
    {
        if (i == 0)
        {
            pipe(fds1);
            runSimpleCommandStartPipe(commands.at(i), fds1);
        }
        else if (i == commands.size() - 1)
        {
            runSimpleCommandEndPipe(commands.at(i), fds1);
            close(fds1[0]);
            close(fds1[1]);
        }
        else
        {
            int fds2[2];

            pipe(fds2);
            runSimpleCommandPipe(commands.at(i), fds1, fds2);
            close(fds1[0]);
            close(fds1[1]);
            fds1[0] = fds2[0];
            fds1[1] = fds2[1];
        }
    }

    for (size_t i = 0; i < commands.size(); i++)
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
        if (checkForRedirections(com) == 0)
        {
            executeCommand(com);
        }
    }
}

void Interpreter::runSimpleCommandEndPipe(const SimpleCommand *com, int fds1[])
{
    if (fork() == 0)
    {
        dup2(fds1[0], 0);
        close(fds1[0]);
        close(fds1[1]);
        if (checkForRedirections(com) == 0)
        {
            executeCommand(com);
        }
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
        if (checkForRedirections(com) == 0)
        {
            executeCommand(com);
        }
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
std::vector<const SimpleCommand *> Interpreter::convertPipeline(const Pipeline *pipeline) {
    std::unique_ptr<PipelineConverter> pipelineSerializer = std::make_unique<PipelineConverter>();
    pipeline->accept(pipelineSerializer.get());
    return pipelineSerializer->getResult();
}
void Interpreter::PipelineConverter::visit(const SimpleCommand *simpleCommand) {
    result.push_back(simpleCommand);
}
void Interpreter::PipelineConverter::visit(const Pipeline *pipeline) {
    pipeline->lhs->accept(this);
    pipeline->rhs->accept(this);
}

int Interpreter::redirectToFile(const Redirection *redirection)
{
    int fd;
    if (redirection->type == RedirectionType::OUT)
    {
        fd = open(redirection->file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd == -1)
        {
            perror("Error creating file");
            return 1;
        }
        dup2(fd, STDOUT_FILENO);
    }
    else
    {
        fd = open(redirection->file.c_str(), O_RDONLY);
        if (fd == -1)
        {
            perror("Error opening file");
            return 1;
        }
        dup2(fd, STDIN_FILENO);
    }
    close(fd);
    return 0;
}

int Interpreter::checkForRedirections(const SimpleCommand* command)
{
    if (!command->redirections.empty())
    {
        return redirectToFile(&command->redirections.at(0));
    }
    return 0;
}