#include "interpreter.hh"

#include "ast.hh"
#include "lexer.hh"
#include "util.h"

#include <algorithm>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/wait.h>

namespace fs = std::filesystem;


void Interpreter::interpret(std::vector<std::unique_ptr<Statement>> statements) {
    for (const auto &statement: statements)
        statement->accept(this);
}

void Interpreter::redirectToFile(const Redirection *redirection) {
    int fd;
    if (redirection->type == RedirectionType::OUT) {
        fd = open(redirection->file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            throw std::runtime_error("Error creating file");
        }
        dup2(fd, STDOUT_FILENO);
    } else {
        fd = open(redirection->file.c_str(), O_RDONLY);
        if (fd == -1) {
            throw std::runtime_error("Error creating file");
        }
        dup2(fd, STDIN_FILENO);
    }
    close(fd);
}

void Interpreter::visit(const SimpleCommand *simpleCommand) {
    const auto transformedCommand = transformCommand(*simpleCommand);

    if (auto builtInCom = getBuiltinCommand(simpleCommand)) {
        (*builtInCom)(*this, simpleCommand);
        return;
    }
    if (fork() == 0) {
        executeCommand(&transformedCommand);
        exit(-1);
    }

    wait(nullptr);
}

void Interpreter::executeCommand(const SimpleCommand *com) {
    if (!com->redirections.empty()) {
        for (const auto &redirection: com->redirections) {
            redirectToFile(&redirection);
        }
    }
    if (com->args.empty()) {
        execlp(com->name.c_str(), com->name.c_str(), NULL);
        exit(-1);
    } else {
        std::vector<const char *> args;
        args.push_back(com->name.c_str());
        for (const auto &arg: com->args) {
            args.push_back(arg.c_str());
        }
        args.push_back(nullptr);
        execvp(com->name.c_str(), const_cast<char *const *>(args.data()));
        exit(-1);
    }
}

std::optional<Interpreter::BuiltInCommand> Interpreter::getBuiltinCommand(const SimpleCommand *com) {
    auto res = std::find_if(builtInCommands.begin(), builtInCommands.end(), [&](const auto &pair) { return pair.first == com->name; });
    if (res != builtInCommands.end())
        return res->second;
    return std::nullopt;
}

auto Interpreter::convertPipeline(const Pipeline *pipeline) {
    PipelineConverter pipelineSerializer;
    pipeline->accept(&pipelineSerializer);
    return pipelineSerializer.getResult();
}

void Interpreter::visit(const Pipeline *pipeline) {
    const auto commands = convertPipeline(pipeline);

    int fds1[2];

    for (size_t i = 0; i < commands.size(); i++) {
        if (i == 0) {
            pipe(fds1);
            runStatementStartInPipe(commands.at(i), fds1);
        } else if (i == commands.size() - 1) {
            runStatementEndInPipe(commands.at(i), fds1);
            close(fds1[0]);
            close(fds1[1]);
        } else {
            int fds2[2];

            pipe(fds2);
            runStatementInPipe(commands.at(i), fds1, fds2);
            close(fds1[0]);
            close(fds1[1]);
            fds1[0] = fds2[0];
            fds1[1] = fds2[1];
        }
    }

    for (size_t i = 0; i < commands.size(); i++) {
        wait(nullptr);
    }
}

void Interpreter::runStatementStartInPipe(const Statement *stmt, int fds[]) {
    if (fork() == 0) {
        dup2(fds[1], 1);
        close(fds[1]);
        close(fds[0]);

        PipelineExecutor executor(this);
        stmt->accept(&executor);
        exit(0);
    }
}

void Interpreter::runStatementEndInPipe(const Statement *stmt, int fds[]) {
    if (fork() == 0) {
        dup2(fds[0], 0);
        close(fds[0]);
        close(fds[1]);

        PipelineExecutor executor(this);
        stmt->accept(&executor);
        exit(0);
    }
}

void Interpreter::runStatementInPipe(const Statement *stmt, int fds1[], int fds2[]) {
    if (fork() == 0) {
        close(fds1[1]);
        close(fds2[0]);
        dup2(fds1[0], 0);
        dup2(fds2[1], 1);
        close(fds1[0]);
        close(fds2[1]);

        PipelineExecutor executor(this);
        stmt->accept(&executor);
        exit(0);
    }
}

void Interpreter::visit(const Assignment *assignment) {
    const auto transformedValue = performWordExpansions(assignment->value);
    variables[assignment->var] = transformedValue;
}

void Interpreter::visit(const Subshell *subshell) {
    if (fork() == 0) {
        for (const auto &red: subshell->redirections) {
            redirectToFile(&red);
        }
        std::istringstream inputStream(subshell->cmds);
        Lexer lexer(&inputStream);
        yy::parser parser(lexer, *this, false);
        parser();
        exit(0);
    }

    wait(nullptr);
}

std::string Interpreter::getSymbol(const std::string &name) {
    auto varValue = variables.find(name);
    if (varValue == variables.end()) {
        auto envValue = getenv(name.c_str());
        if (envValue == nullptr) return "";
        return getenv(name.c_str());
    }
    return varValue->second;
}
std::string Interpreter::performWordExpansions(const std::string &text) {
    bool inQuote = false;
    auto dollarExpressionStart = text.end();
    std::string newText;

    for (auto it = text.begin(); it != text.end(); ++it) {
        if (dollarExpressionStart != text.end()) {
            if (isalnum(*it)) {
                continue;
            } else {
                newText += getSymbol(std::string(dollarExpressionStart, it));
                dollarExpressionStart = text.end();
            }
        }
        if (*it == '\'')
            inQuote = !inQuote;
        else if (*it == '$' && !inQuote)
            dollarExpressionStart = it + 1;
        else
            newText += *it;
    }

    if (dollarExpressionStart != text.end())
        newText += getSymbol(std::string(dollarExpressionStart, text.end()));
    if (inQuote)
        throw std::runtime_error("Unmatched quote: " + text);

    return newText;
}

SimpleCommand Interpreter::transformCommand(const SimpleCommand &com) {
    SimpleCommand transformedCommand(com);
    transformedCommand.name = performWordExpansions(com.name);

    const auto argTransformation = [this](auto text) { return this->performWordExpansions(text); };
    std::transform(com.args.begin(), com.args.end(), transformedCommand.args.begin(), argTransformation);
    return transformedCommand;
}

void Interpreter::prompt() {
    std::cout << getSymbol("USER") << ":" << shortPwd() << "$ ";
}

std::string Interpreter::shortPwd() {
    fs::path pwd(getSymbol("PWD"));
    fs::path home(getSymbol("HOME"));
    if (!isSubDir(pwd, home)) return pwd.string();
    if (pwd == home) return "~";
    return "~/" + fs::relative(pwd, home).string();
}

void Interpreter::PipelineConverter::visit(const SimpleCommand *simpleCommand) {
    result.push_back(simpleCommand);
}

void Interpreter::PipelineConverter::visit(const Pipeline *pipeline) {
    pipeline->lhs->accept(this);
    pipeline->rhs->accept(this);
}

void Interpreter::PipelineConverter::visit(const Subshell *subshell) {
    result.push_back(subshell);
}

void Interpreter::PipelineConverter::visit(const Assignment *assignment) {
    result.push_back(assignment);
}


void Interpreter::changeDirectory(const SimpleCommand *com) {
    if (com->args.size() > 1) throw std::runtime_error("Expected no more than one argument");
    auto path = com->args.empty() ? getenv("HOME") : com->args.at(0).c_str();
    if (chdir(path))
        throw std::runtime_error("cd failed with error code: " + std::to_string(errno));
    char buf[4096];
    auto absPath = getcwd(buf, sizeof(buf));
    if (!absPath)
        throw std::runtime_error("getcwd failed");
    if (setenv("PWD", absPath, true))
        throw std::runtime_error("PWD failed with error code: " + std::to_string(errno));
}


void Interpreter::runScript(const SimpleCommand *com) {
    if (com->args.size() != 1)
        throw std::runtime_error("Expected file name");
    const auto &fileName = com->args.at(0);
    std::ifstream script(fileName);
    if (!script)
        throw std::runtime_error("Error: Could not open file " + fileName);
    Lexer lexer(&script);
    yy::parser parser(lexer, *this, false);

    parser();
}

void Interpreter::exportVar(const SimpleCommand *com) {
    if (com->args.size() != 1) throw std::runtime_error("Expected var name");
    setenv(com->args[0].c_str(), getSymbol(com->args[0]).c_str(), true);
}

const std::initializer_list<std::pair<std::string, std::function<void(Interpreter &, const SimpleCommand *com)>>> Interpreter::builtInCommands{
        {"cd", [](Interpreter &interpreter, auto com) { interpreter.changeDirectory(com); }},
        {".", [](Interpreter &interpreter, auto com) { interpreter.runScript(com); }},
        {"export", [](Interpreter &interpreter, auto com) { interpreter.exportVar(com); }},
};

void Interpreter::PipelineExecutor::visit(const SimpleCommand *simpleCommand) {
    const auto transformedCommand = interpreter->transformCommand(*simpleCommand);

    if (auto builtInCom = interpreter->getBuiltinCommand(&transformedCommand)) {
        (*builtInCom)(*interpreter, &transformedCommand);
        exit(0);
    }
    executeCommand(&transformedCommand);
}

void Interpreter::PipelineExecutor::visit(const Pipeline *pipeline) {
    throw std::runtime_error("Nested pipeline. That should not be possible after flattening");
}

void Interpreter::PipelineExecutor::visit(const Assignment *assignment) {
    interpreter->visit(assignment);
}

void Interpreter::PipelineExecutor::visit(const Subshell *subshell) {
    for (const auto &red: subshell->redirections) {
        redirectToFile(&red);
    }
    std::istringstream inputStream(subshell->cmds);
    Lexer lexer(&inputStream);
    yy::parser parser(lexer, *interpreter, false);
    parser();
}
