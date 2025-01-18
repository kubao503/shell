#include "interpreter.hh"

#include "ast.hh"
#include "lexer.hh"
#include "util.h"

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/wait.h>
#include <algorithm>
#include <filesystem>

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
    SimpleCommand transformedCommand(*simpleCommand);
    transformCommand(transformedCommand);
    if (executeBuiltinCommand(&transformedCommand))
        return;
    if (fork() == 0) {
        executeCommand(&transformedCommand);
    }

    wait(nullptr);
}

void Interpreter::executeCommand(const SimpleCommand *com) {
    if (!com->redirections.empty()) {
        for (const auto& redirection: com->redirections) {
            redirectToFile(&redirection);
        }
    }
    if (com->args.empty()) {
        execlp(com->name.c_str(), com->name.c_str(), NULL);
    } else {
        std::vector<const char *> args;
        args.push_back(com->name.c_str());
        for (const auto &arg: com->args) {
            args.push_back(arg.c_str());
        }
        args.push_back(nullptr);
        execvp(com->name.c_str(), const_cast<char *const *>(args.data()));
    }
}

bool Interpreter::executeBuiltinCommand(const SimpleCommand *com) {
    if (com->name == "cd") {
        if (com->args.size() > 1)
            throw std::runtime_error("Expected no more than one argument");
        auto path = com->args.empty() ? getenv("HOME") : com->args.at(0).c_str();
        if (chdir(path))
            throw std::runtime_error("cd failed with error code: " + std::to_string(errno));
        char buf[4096];
        auto absPath = getcwd(buf, sizeof(buf));
        if (!absPath)
            throw std::runtime_error("getcwd failed");
        if (setenv("PWD", absPath, true))
            throw std::runtime_error("PWD failed with error code: " + std::to_string(errno));

        return true;
    } else if (com->name == ".") {
        if (com->args.size() != 1)
            throw std::runtime_error("Expected file name");
        const auto &fileName = com->args.at(0);
        std::ifstream script(fileName);
        if (!script)
            throw std::runtime_error("Error: Could not open file " + fileName);
        Lexer lexer(&script);
        yy::parser parser(lexer, *this, false);

        parser();

        return true;
    } else if (com->name == "export") {
        if (com->args.size() != 1) throw std::runtime_error("Expected var name");
        setenv(com->args[0].c_str(), getSymbol(com->args[0]).c_str(), true);
    }
    return false;
}

void Interpreter::visit(const Pipeline *pipeline) {
    std::vector<const SimpleCommand *> commands = convertPipeline(pipeline);
    std::vector<SimpleCommand> transformedCommands;
    for(const auto command : commands) {
        SimpleCommand transformedCommand(*command);
        transformCommand(transformedCommand);
        transformedCommands.push_back(std::move(transformedCommand));
    }

    int fds1[2];

    for (size_t i = 0; i < transformedCommands.size(); i++) {
        if (i == 0) {
            pipe(fds1);
            runSimpleCommandStartPipe(&transformedCommands.at(i), fds1);
        } else if (i == commands.size() - 1) {
            runSimpleCommandEndPipe(&transformedCommands.at(i), fds1);
            close(fds1[0]);
            close(fds1[1]);
        } else {
            int fds2[2];

            pipe(fds2);
            runSimpleCommandPipe(&transformedCommands.at(i), fds1, fds2);
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

void Interpreter::runSimpleCommandStartPipe(const SimpleCommand *com, int fds1[]) {
    if (fork() == 0) {
        dup2(fds1[1], 1);
        close(fds1[1]);
        close(fds1[0]);
        if (executeBuiltinCommand(com))
            exit(0);
        executeCommand(com);
    }

    wait(nullptr);
}

void Interpreter::runSimpleCommandEndPipe(const SimpleCommand *com, int fds1[]) {
    if (fork() == 0) {
        dup2(fds1[0], 0);
        close(fds1[0]);
        close(fds1[1]);
        if (executeBuiltinCommand(com))
            exit(0);
        executeCommand(com);
    }
}

void Interpreter::runSimpleCommandPipe(const SimpleCommand *com, int fds1[], int fds2[]) {
    if (fork() == 0) {
        close(fds1[1]);
        close(fds2[0]);
        dup2(fds1[0], 0);
        dup2(fds2[1], 1);
        close(fds1[0]);
        close(fds2[1]);
        if (executeBuiltinCommand(com))
            exit(0);
        executeCommand(com);
    }
}

void Interpreter::visit(const Assignment *assignment) {
    Assignment transformedAssignment(*assignment);
    performWordExpansions(transformedAssignment.value);
    variables[transformedAssignment.var] = transformedAssignment.value;
}
void Interpreter::visit(const Subshell *subshell) {
    if (fork() == 0) {
        for(const auto& red : subshell->redirections) {
            redirectToFile(&red);
        }
        std::istringstream inputStream(subshell->cmds);
        Lexer lexer(&inputStream);
        yy::parser parser(lexer, *this, false);
        parser();
    }

    wait(nullptr);
}
std::vector<const SimpleCommand *> Interpreter::convertPipeline(const Pipeline *pipeline) {
    std::unique_ptr<PipelineConverter> pipelineSerializer = std::make_unique<PipelineConverter>();
    pipeline->accept(pipelineSerializer.get());
    return pipelineSerializer->getResult();
}
std::string Interpreter::getSymbol(const std::string& name) {
    auto varValue = variables.find(name);
    if(varValue == variables.end()) {
        auto envValue = getenv(name.c_str());
        if(envValue == nullptr) return "";
        return getenv(name.c_str());
    }
    return varValue->second;
}
void Interpreter::performWordExpansions(std::string &text) {
    bool inQuote = false;
    bool inDollarExpression = false;
    std::string varName;
    std::string newText;
    int i = 0;
    while(i < text.length()) {
        char c = text.at(i);
        if(inDollarExpression) {
            if(isalnum(c)) {varName += c; i++;}
            else {
                newText += getSymbol(varName);
                inDollarExpression = false;
                varName = "";
            }
            continue;
        }
        if(c == '\'') inQuote = !inQuote;
        if(c == '$' && !inQuote) inDollarExpression = true;
        else newText += c;
        i++;
    }
    if(inDollarExpression) {
        newText += getSymbol(varName);
    }
    text = newText;
    text.erase(std::remove(text.begin(), text.end(), '\''), text.end());
}
void Interpreter::transformCommand(SimpleCommand &com) {
    performWordExpansions(com.name);
    for(auto &arg : com.args) {
        performWordExpansions(arg);
    }
}
void Interpreter::prompt() {
    std::cout << getSymbol("USER") << ":" << shortPwd() << "$ ";
}
std::string Interpreter::shortPwd() {
    fs::path pwd(getSymbol("PWD"));
    fs::path home(getSymbol("HOME"));
    if(!isSubDir(pwd, home)) return pwd.string();
    if(pwd == home) return "~";
    return "~/" + fs::relative(pwd, home).string();
}
void Interpreter::PipelineConverter::visit(const SimpleCommand *simpleCommand) {
    result.push_back(simpleCommand);
}
void Interpreter::PipelineConverter::visit(const Pipeline *pipeline) {
    pipeline->lhs->accept(this);
    pipeline->rhs->accept(this);
}
