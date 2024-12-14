#ifndef AST_H
#define AST_H

#include <string>
#include <vector>

struct Statement
{
    virtual ~Statement() = default;
};

struct Assignment
{
    std::string var;

    Assignment(std::string var) : var{std::move(var)} {}
};

enum class RedirectionType
{
    IN,
    OUT
};

struct Redirection
{
    std::string file;
    RedirectionType type;
};

struct SimpleCommand : public Statement
{
    std::string name;
    std::vector<Assignment *> assignments;
    std::vector<std::string> args;
    std::vector<Redirection *> redirections;
    SimpleCommand(std::string name,
                  std::vector<Assignment *> assignments = {},
                  std::vector<std::string> args = {},
                  std::vector<Redirection *> redirections = {})
        : name(std::move(name)),
          assignments(std::move(assignments)),
          args(std::move(args)),
          redirections(std::move(redirections)) {}
    ~SimpleCommand() override
    {
        for (auto assignment : assignments)
            delete assignment;
        for (auto redirection : redirections)
            delete redirection;
    }
};

struct SubProgram : public Statement
{
    std::string cmds;

    SubProgram(std::string cmds) : cmds{std::move(cmds)} {}
};

struct Pipeline : public Statement
{
    Statement *lhs;
    Statement *rhs;

    Pipeline(Statement *lhs, Statement *rhs) : lhs{lhs}, rhs{rhs} {}
    ~Pipeline() override
    {
        delete lhs;
        delete rhs;
    }
};

#endif
