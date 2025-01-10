#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser_output.hh"
#include "ast.hh"

class Interpreter : public ParserOutput
{
    void interpretStatement(const Statement *statement, int pipelineSize=-1, int pipelineStep=-1);
    void interpret(std::vector<std::unique_ptr<Statement>> statements);
    void runSimpleCommand(const SimpleCommand *com);
    void runSimpleCommandStartPipe(const SimpleCommand *com, int fds1[], int fds2[]);
    void runSimpleCommandPipe(const SimpleCommand *com, int fds1[], int fds2[]);
    void runSimpleCommandEndPipe(const SimpleCommand *com, int fds1[], int fds2[]);
    void createVectorFromPipeline(const Statement* statement, std::vector<const SimpleCommand*> *vecotrOfCommands);
    void executeCommandArgs(const SimpleCommand *com);
    void executeCommand(const SimpleCommand *com);
    void runPipeline(const Statement *statement);
};

#endif
