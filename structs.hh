#include <string>
#include <vector>

class Command
{
};

class Redirection
{
    std::string file;
    std::string type;
};

struct Program
{
    std::string name;
    std::vector<std::string> args;
    std::vector<Redirection> redirections;
};

class Pipeline : public Command
{
    std::vector<Program> programs;
};

class Assignment : public Command
{
    std::string var;
    std::string value;
    bool exp;
};

class SubProgram : public Command
{
    std::string cmds;
};
