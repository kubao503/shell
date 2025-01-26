#include "interpreter.hh"

#include "ast.hh"
#include "lexer.hh"
#include "pipeline_converter.hh"
#include "util.h"

#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/wait.h>

namespace fs = std::filesystem;


void Interpreter::interpret(std::vector<std::unique_ptr<Statement>> statements) {
    for (const auto &statement: statements) {
        try {
            statement->accept(this);
        } catch(InterpreterError& e) {
            this->exitCode = e.getExitCode();
            std::cerr << e.what() << "\n";
        }
    }
}

void Interpreter::redirectToFile(const Redirection *redirection) {
    int fd;
    if (redirection->type == RedirectionType::OUT) {
        fd = open(redirection->file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd == -1) throw InterpreterError("Error creating file", errno);
        if(dup2(fd, STDOUT_FILENO) == -1)
            throw InterpreterError("Descriptor duplication failed 1", errno);
    } else {
        fd = open(redirection->file.c_str(), O_RDONLY);
        if (fd == -1)
            throw InterpreterError("Error creating file", errno);
        if(dup2(fd, STDIN_FILENO) == -1) throw InterpreterError("Descriptor duplication failed", errno);
    }
    if(close(fd)) throw InterpreterError("Failed to close descriptor", errno);
}

void Interpreter::visit(const SimpleCommand *simpleCommand) {
    const auto transformedCommand = transformCommand(*simpleCommand);

    if (auto builtInCom = getBuiltinCommand(simpleCommand)) {
        (*builtInCom)(*this, simpleCommand);
        return;
    }
    int forkReturn = fork();
    if (forkReturn == 0) {
        try {
            executeCommand(&transformedCommand);
        } catch (InterpreterError& e) {
            std::cerr << e.what() << "\n";
            exit(e.getExitCode());
        }
        exit(-1);
    } else if (forkReturn == -1) {
        throw InterpreterError("Process creation failed", errno);
    }
    // PID of the created process can be safely ignored
    int exitStatus = 0;
    int waitReturn = wait(&exitStatus);
    if(waitReturn == -1) {
        throw InterpreterError("Wait failed", errno);
    }
    this->exitCode = WEXITSTATUS(exitStatus);
}

void Interpreter::executeCommand(const SimpleCommand *com) {
    if (!com->redirections.empty()) {
        for (const auto &redirection: com->redirections) {
            redirectToFile(&redirection);
        }
    }
    if (com->args.empty()) {
        if (execlp(com->name.c_str(), com->name.c_str(), NULL))
            throw InterpreterError("Failed to execute command " + com->name, errno);
        exit(-1);
    } else {
        std::vector<const char *> args;
        args.push_back(com->name.c_str());
        for (const auto &arg: com->args) {
            args.push_back(arg.c_str());
        }
        args.push_back(nullptr);
        if (execvp(com->name.c_str(), const_cast<char *const *>(args.data())) == -1)
            throw InterpreterError("Failed to execute command " + com->name, errno);
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
    std::vector<int> pids;
    for (size_t i = 0; i < commands.size(); i++) {
        if (i == 0) {
            if(pipe(fds1) == -1)
                throw InterpreterError("Pipe creation failed", errno);
            int forkReturn = fork();
            if(forkReturn == 0) {
                try {
                    runStatementStartInPipe(commands.at(i), fds1);
                } catch (InterpreterError& e) {
                    std::cerr << e.what() << "\n";
                    exit(e.getExitCode());
                }
                exit(-1);
            }
            else if (forkReturn == -1) throw InterpreterError("Process creation failed", errno);
            pids.push_back(forkReturn);
        } else if (i == commands.size() - 1) {
            int forkReturn = fork();
            if(forkReturn == 0) {
                try {
                    runStatementEndInPipe(commands.at(i), fds1);
                } catch (InterpreterError& e) {
                    std::cerr << e.what() << "\n";
                    exit(e.getExitCode());
                }
                exit(-1);
            }
            else if (forkReturn == -1) throw InterpreterError("Process creation failed", errno);
            pids.push_back(forkReturn);
            if(close(fds1[0]) == -1) throw InterpreterError("Failed to close descriptor", errno);
            if(close(fds1[1]) == -1) throw InterpreterError("Failed to close descriptor", errno);
        } else {
            int fds2[2];
            if(pipe(fds2) == -1) throw InterpreterError("Pipe creation failed", errno);
            int forkReturn = fork();
            if(forkReturn == 0) {
                try {
                    runStatementInPipe(commands.at(i), fds1, fds2);
                } catch (InterpreterError& e) {
                    std::cerr << e.what() << "\n";
                    exit(e.getExitCode());
                }
                exit(-1);
            }
            else if (forkReturn == -1) throw InterpreterError("Process creation failed", errno);
            pids.push_back(forkReturn);
            if(close(fds1[0]) == -1) throw InterpreterError("Failed to close descriptor", errno);
            if(close(fds1[1]) == -1) throw InterpreterError("Failed to close descriptor", errno);
            fds1[0] = fds2[0];
            fds1[1] = fds2[1];
        }
    }
    int lastExitStatus = 0;
    for (int pid : pids) {
        waitpid(pid, &lastExitStatus, 0);
    }
    this->exitCode = WEXITSTATUS(lastExitStatus);
}

void Interpreter::runStatementStartInPipe(const Statement *stmt, int fds[]) {
    if(dup2(fds[1], 1) == -1) throw InterpreterError("Descriptor duplication failed", errno);
    if(close(fds[1]) == -1) throw InterpreterError("Failed to close descriptor", errno);
    if(close(fds[0]) == -1) throw InterpreterError("Failed to close descriptor", errno);

    PipelineExecutor executor(this);
    stmt->accept(&executor);
    exit(0);
}

void Interpreter::runStatementEndInPipe(const Statement *stmt, int fds[]) {
    if(dup2(fds[0], 0) == -1) throw InterpreterError("Descriptor duplication failed", errno);
    if(close(fds[0]) == -1) throw InterpreterError("Failed to close descriptor", errno);
    if(close(fds[1]) == -1) throw InterpreterError("Failed to close descriptor", errno);

    PipelineExecutor executor(this);
    stmt->accept(&executor);
    exit(0);
}

void Interpreter::runStatementInPipe(const Statement *stmt, int fds1[], int fds2[]) {
    if(close(fds1[1]) == -1) throw InterpreterError("Failed to close descriptor", errno);
    if(close(fds2[0]) == -1) throw InterpreterError("Failed to close descriptor", errno);
    if(dup2(fds1[0], 0) == -1) throw InterpreterError("Descriptor duplication failed", errno);
    if(dup2(fds2[1], 1) == -1) throw InterpreterError("Descriptor duplication failed", errno);
    if(close(fds1[0]) == -1) throw InterpreterError("Failed to close descriptor", errno);
    if(close(fds2[1]) == -1) throw InterpreterError("Failed to close descriptor", errno);

    PipelineExecutor executor(this);
    stmt->accept(&executor);
    exit(0);
}

void Interpreter::PipelineExecutor::visit(const SimpleCommand *simpleCommand) {
    const auto transformedCommand = interpreter->transformCommand(*simpleCommand);

    if (auto builtInCom = Interpreter::getBuiltinCommand(&transformedCommand)) {
        (*builtInCom)(*interpreter, &transformedCommand);
        exit(0);
    }
    executeCommand(&transformedCommand);
}

void Interpreter::PipelineExecutor::visit(const Pipeline *pipeline) {
    throw InterpreterError("Nested pipeline. That should not be possible after flattening");
}

void Interpreter::PipelineExecutor::visit(const Assignment *assignment) {
    assignment->accept(interpreter);
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

void Interpreter::visit(const Assignment *assignment) {
    const auto transformedValue = performWordExpansions(assignment->value);
    variables[assignment->var] = transformedValue;
}

void Interpreter::visit(const Subshell *subshell) {
    int forkReturn = fork();
    if (forkReturn == 0) {
        try {
            for (const auto &red: subshell->redirections) {
                redirectToFile(&red);
            }
            std::istringstream inputStream(subshell->cmds);
            Lexer lexer(&inputStream);
            Interpreter interpreter(*this);
            yy::parser parser(lexer, interpreter, false);
            parser();
            exit(interpreter.exitCode);
        } catch (InterpreterError& e) {
            std::cerr << e.what() << "\n";
            exit(e.getExitCode());
        }
    } else if (forkReturn == -1) {
        throw InterpreterError("Process creation failed", errno);
    }
    int exitStatus = 0;
    int waitReturn = wait(&exitStatus);
    if(waitReturn == -1) {
        throw InterpreterError("Wait failed", errno);
    }
    this->exitCode = WEXITSTATUS(exitStatus);
}

std::string Interpreter::getSymbol(const std::string &name) {
    if(name == "?") return std::to_string(this->exitCode);
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
            if (isalnum(*it) || *it == '?') {
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
        throw InterpreterError("Unmatched quote: " + text);

    return newText;
}

SimpleCommand Interpreter::transformCommand(const SimpleCommand &com) {
    SimpleCommand transformedCommand(com);
    transformedCommand.name = performWordExpansions(com.name);

    const auto argTransformation = [this](const auto& text) { return this->performWordExpansions(text); };
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

void Interpreter::changeDirectory(const SimpleCommand *com) {
    if (com->args.size() > 1) throw std::runtime_error("Expected no more than one argument");
    auto path = com->args.empty() ? getenv("HOME") : com->args.at(0).c_str();
    if (chdir(path) == -1)
        throw InterpreterError("cd failed", errno);
    char buf[4096];
    auto absPath = getcwd(buf, sizeof(buf));
    if (!absPath)
        throw InterpreterError("getcwd failed", errno);
    if (setenv("PWD", absPath, true) == -1)
        throw InterpreterError("PWD failed", errno);
}


void Interpreter::runScript(const SimpleCommand *com) {
    if (com->args.size() != 1)
        throw InterpreterError("Expected file name");
    const auto &fileName = com->args.at(0);
    std::ifstream script(fileName);
    if (!script)
        throw InterpreterError("Error: Could not open file " + fileName);
    Lexer lexer(&script);
    yy::parser parser(lexer, *this, false);

    parser();
}

void Interpreter::exportVar(const SimpleCommand *com) {
    if (com->args.size() != 1) throw InterpreterError("Expected var name");
    if(setenv(com->args[0].c_str(), getSymbol(com->args[0]).c_str(), true))
        throw InterpreterError("setenv failed", errno);
}

const std::initializer_list<std::pair<std::string, std::function<void(Interpreter &, const SimpleCommand *com)>>> Interpreter::builtInCommands{
        {"cd", [](Interpreter &interpreter, auto com) { Interpreter::changeDirectory(com); }},
        {".", [](Interpreter &interpreter, auto com) { interpreter.runScript(com); }},
        {"export", [](Interpreter &interpreter, auto com) { interpreter.exportVar(com); }},
};
const char * InterpreterError::what() const noexcept {
    return whatMsg.c_str();
}
