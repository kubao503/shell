#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <stdexcept>
#include <memory>

struct Statement
{
    virtual ~Statement() = default;
};

struct Assignment : public Statement
{
    std::string var;
    std::string value;

    Assignment(std::string str)
    {
        auto index = str.find('=');
        if (index == str.npos)
            throw std::runtime_error("No '=' sign passed to Assignment ctor: " + str);
        var = str.substr(0, index);
        value = str.substr(index + 1);
    }
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
    std::vector<std::string> args;
    std::vector<Redirection> redirections;
    SimpleCommand(std::string name = "",
                  std::vector<Assignment *> assignments = {},
                  std::vector<std::string> args = {},
                  std::vector<Redirection> redirections = {})
        : name(std::move(name)),
          args(std::move(args)),
          redirections(std::move(redirections)) {}
};

struct SubShell : public Statement
{
    std::string cmds;
    std::vector<Redirection> redirections;

    SubShell(std::string cmds, std::vector<Redirection> redirections = {})
        : cmds{std::move(cmds)}, redirections{std::move(redirections)} {}
};

struct Pipeline : public Statement
{
    std::unique_ptr<Statement> lhs;
    std::unique_ptr<Statement> rhs;

    Pipeline(std::unique_ptr<Statement> lhs, std::unique_ptr<Statement> rhs) : lhs{std::move(lhs)}, rhs{std::move(rhs)} {}
};

#endif
