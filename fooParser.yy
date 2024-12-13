%require "3.2"
%language "c++"
%define api.token.constructor

%code requires {
    #include <string>
    #include "structs.hh"

    class FooLexer;
}

%code top {
    #include "fooParser.tab.hh"
    #include "FooLexer.hh"

    namespace yy {
        static parser::symbol_type yylex(FooLexer &lexer) {
            return lexer.get_next_token();
        }
    }

    void interpret(std::vector<Command *> commands);
}

%define api.value.type variant
%lex-param {FooLexer &lexer}
%parse-param {FooLexer &lexer}

%header

%token  <std::string> WORD
%token  <std::string> ASSIGNMENT_WORD

%type   <std::vector<Command*>> list
%type   <Command*> command;
%type   <Command*> pipeline
%type   <Program*> program
%type   <Command*> assignment
%type   <Program*> cmd_suffix
%type   <Redirection*> io_redirect

%%

file             : linebreak complete_commands linebreak
                 | linebreak
                 ;
complete_commands: complete_commands newline_list complete_command
                 |                                complete_command
                 ;
complete_command : list ';'                 { interpret($list); }
                 | list                     { interpret($list); }
                 ;
list             : list ';' command         {
                                                $1.push_back($command);
                                                $$ = $1;
                                            }
                 |          command         { $$ = std::vector<Command*>{$command}; }
                 ;
command          : pipeline
                 | subshell
                 | assignment
                 ;
pipeline         :              program     { $$ = new Pipeline{{$program}}; }
                 | pipeline '|' program     {
                                                static_cast<Pipeline*>($1)->programs.push_back($program);
                                                $$ = $1;
                                            }
                 ;
subshell         : '(' compound_list ')'
                 ;
compound_list    : linebreak term
                 | linebreak term separator
                 ;
term             : term separator command
                 |                command
                 ;
program          : WORD cmd_suffix          {
                                                $cmd_suffix->name = $WORD;
                                                $$ = $cmd_suffix;
                                            }
                 | WORD                     { $$ = new Program{$WORD, {}, {}}; }
                 ;
assignment       : ASSIGNMENT_WORD          { $$ = new Assignment{$ASSIGNMENT_WORD}; }
                 ;
cmd_suffix       :            io_redirect   { $$ = new Program{"", {}, {$io_redirect}}; }
                 | cmd_suffix io_redirect   { $1->redirections.push_back($io_redirect); $$ = $1; }
                 |            WORD          { $$ = new Program{"", {$WORD}, {}}; }
                 | cmd_suffix WORD          { $1->args.push_back($WORD); $$ = $1; }
                 ;
io_redirect      : '<' WORD                 { $$ = new Redirection{$WORD, RedirectionType::IN}; }
                 | '>' WORD                 { $$ = new Redirection{$WORD, RedirectionType::OUT}; }
                 ;
newline_list     :              '\n'
                 | newline_list '\n'
                 ;
linebreak        : newline_list
                 | /* empty */
                 ;
separator        : ';' linebreak
                 | newline_list
                 ;


%%

void yy::parser::error(const std::string &message)
{
    std::cerr << "Error: " << message << std::endl;
}
