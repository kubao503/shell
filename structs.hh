#include <string>
#include <vector>

struct Statement
{
    virtual ~Statement() = default;
};

struct Assignment : public Statement
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
};
