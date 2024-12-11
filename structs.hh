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

struct Pipeline
{
    std::vector<Program*> programs;
};

struct Assignment
{
    std::string var;
};

class SubProgram : public Command
{
    std::string cmds;
};
