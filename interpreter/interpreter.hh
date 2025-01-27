#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.hh"
#include "parser_output.hh"
#include <cstring>
#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>

class InterpreterError : public std::exception {
private:
    int exitCode;
    std::string message;
    std::string whatMsg;
public:
    InterpreterError(const std::string& message, int errNum) : message(message),
                                                          exitCode(errNum),
                                                          whatMsg(message + " (" + std::string(strerror(errNum)) + ")") {};
    explicit InterpreterError(const std::string& message) : message(message), exitCode(1), whatMsg(message) {};
    [[nodiscard]] int getExitCode() const {return exitCode;}
    [[nodiscard]] std::string getMessage() const {return message;}
    [[nodiscard]] const char * what() const noexcept override;
};

class Interpreter : public ParserOutput, StatementVisitor {
    struct PipelineExecutor : public StatementVisitor {
        Interpreter *interpreter;

        explicit PipelineExecutor(Interpreter *interpreter) : interpreter{interpreter} {}

        void visit(const SimpleCommand *simpleCommand) override;
        void visit(const Pipeline *pipeline) override;
        void visit(const Assignment *assignment) override;
        void visit(const Subshell *subshell) override;
    };

public:
    void interpret(std::vector<std::unique_ptr<Statement>> statements) override;
    void prompt() override;
    std::string performWordExpansions(const std::string &text);
    int getExitCode() const {return this->exitCode;}
private:
    using BuiltInCommand = std::function<void(Interpreter &, const SimpleCommand *)>;
    static const std::initializer_list<std::pair<std::string, BuiltInCommand>> builtInCommands;
    std::unordered_map<std::string, std::string> variables;
    int exitCode;

    std::string getSymbol(const std::string &name);

    void runStatementStartInPipe(const Statement *stmt, int fds1[]);
    void runStatementInPipe(const Statement *stmt, int fds1[], int fds2[]);
    void runStatementEndInPipe(const Statement *stmt, int fds1[]);
    static auto convertPipeline(const Pipeline *pipeline);
    static void executeCommand(const SimpleCommand *com);
    static std::optional<BuiltInCommand> getBuiltinCommand(const SimpleCommand *com);
    void visit(const SimpleCommand *simpleCommand) override;
    void visit(const Assignment *assignment) override;
    void visit(const Pipeline *pipeline) override;
    void visit(const Subshell *subshell) override;
    static void redirectToFile(const Redirection *redirection);
    SimpleCommand transformCommand(const SimpleCommand &com);
    std::string shortPwd();

    void changeDirectory(const SimpleCommand *com);
    void runScript(const SimpleCommand *com);
    void exportVar(const SimpleCommand *com);
    void pwd(const SimpleCommand *com);
    void echo(const SimpleCommand *com);
};

#endif
