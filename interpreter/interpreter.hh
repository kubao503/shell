#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.hh"
#include "parser_output.hh"

class Interpreter : public ParserOutput, StatementVisitor {
    class PipelineConverter : public StatementVisitor {
    private:
        std::vector<const SimpleCommand *> result;

    public:
        std::vector<const SimpleCommand *> getResult() { return result; }
        void visit(const Subshell *subshell) override {};
        void visit(const Assignment *assignment) override {};
        void visit(const SimpleCommand *simpleCommand) override;
        void visit(const Pipeline *pipeline) override;
    };
    void interpret(std::vector<std::unique_ptr<Statement>> statements) override;
    static void runSimpleCommandStartPipe(const SimpleCommand *com, int fds1[]);
    static void runSimpleCommandPipe(const SimpleCommand *com, int fds1[], int fds2[]);
    static void runSimpleCommandEndPipe(const SimpleCommand *com, int fds1[]);
    static std::vector<const SimpleCommand *> convertPipeline(const Pipeline *pipeline);
    static void executeCommand(const SimpleCommand *com);
    static bool executeBuiltinCommand(const SimpleCommand *com);

    void visit(const SimpleCommand *simpleCommand) override;
    void visit(const Assignment *assignment) override;
    void visit(const Pipeline *pipeline) override;
    void visit(const Subshell *subshell) override;
    static void redirectToFile(const Redirection *redirection);
};

#endif
