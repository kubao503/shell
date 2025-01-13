#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <stdexcept>
#include <memory>

struct Assignment;
struct SimpleCommand;
struct Subshell;
struct Pipeline;


class StatementVisitor {
public:
    virtual void visit(const Assignment* assignment) = 0;
    virtual void visit(const SimpleCommand* simpleCommand) = 0;
    virtual void visit(const Subshell* subshell) = 0;
    virtual void visit(const Pipeline* pipeline) = 0;
};

struct Statement
{
    virtual ~Statement() = default;
    virtual void accept(StatementVisitor* visitor) const = 0;
};

struct Assignment : public Statement
{
    std::string var;
    std::string value;

    explicit Assignment(const std::string& str)
    {
        auto index = str.find('=');
        if (index == std::string::npos)
            throw std::runtime_error("No '=' sign passed to Assignment ctor: " + str);
        var = str.substr(0, index);
        value = str.substr(index + 1);
    }
    void accept(StatementVisitor *visitor) const override {
        visitor->visit(this);
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
    explicit SimpleCommand(std::string name = "",
                  std::vector<std::string> args = {},
                  std::vector<Redirection> redirections = {})
        : name(std::move(name)),
          args(std::move(args)),
          redirections(std::move(redirections)) {}
    void accept(StatementVisitor *visitor) const override {
        visitor->visit(this);
    }
};

struct Subshell : public Statement
{
    std::string cmds;
    std::vector<Redirection> redirections;

    explicit Subshell(std::string cmds, std::vector<Redirection> redirections = {})
        : cmds{std::move(cmds)}, redirections{std::move(redirections)} {}
    void accept(StatementVisitor *visitor) const override {
        visitor->visit(this);
    }
};

struct Pipeline : public Statement
{
    std::unique_ptr<Statement> lhs;
    std::unique_ptr<Statement> rhs;

    Pipeline(std::unique_ptr<Statement> lhs, std::unique_ptr<Statement> rhs) : lhs{std::move(lhs)}, rhs{std::move(rhs)} {}
    void accept(StatementVisitor *visitor) const override {
        visitor->visit(this);
    }
};

#endif
