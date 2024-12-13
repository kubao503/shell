#include <string>
#include <vector>

struct Command
{
    virtual ~Command() = default;
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

struct Program
{
    std::string name;
    std::vector<std::string> args;
    std::vector<Redirection *> redirections;
};

struct Pipeline : public Command
{
    std::vector<Program *> programs;

    Pipeline(std::vector<Program *> programs) { this->programs = std::move(programs); }
};

struct Assignment : public Command
{
    std::string var;

    Assignment(std::string var) { this->var = std::move(var); }
};

struct SubProgram : public Command
{
    std::string cmds;
};
