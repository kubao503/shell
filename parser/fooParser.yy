%require "3.2"
%language "c++"
%define api.token.constructor

%code requires {
    #include <string>
    #include "structs.hh"

    class FooLexer;
}

%code top {
    #include "parser.hh"
    #include "FooLexer.hh"

    namespace yy {
        static parser::symbol_type yylex(FooLexer &lexer) {
            return lexer.get_next_token();
        }
    }

    void interpret(std::vector<Statement *> statements);
}

%define api.value.type variant
%lex-param {FooLexer &lexer}
%parse-param {FooLexer &lexer}

%header

%token  <std::string> WORD ASSIGNMENT_WORD SUBSHELL

%nterm  <std::vector<Statement*>> list
%nterm  <Statement*>            pipeline command simple_command
%nterm  <Redirection*> io_redirect
%nterm  <std::string> cmd_name cmd_word
%nterm  <SimpleCommand*> cmd_prefix cmd_suffix

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
list             : list ';' pipeline        {
                                                $1.push_back($pipeline);
                                                $$ = $1;
                                            }
                 |          pipeline        { $$ = std::vector<Statement*>{$pipeline}; }
                 ;
pipeline         :                        command
                 | pipeline '|' linebreak command     { $$ = new Pipeline($1, $command); }
                 ;
command          : simple_command
                 | SUBSHELL                 { $$ = new SubProgram($SUBSHELL); }
                 ;
simple_command   : cmd_prefix cmd_word cmd_suffix
                 | cmd_prefix cmd_word
                 | cmd_prefix
                 | cmd_name cmd_suffix { $cmd_suffix->name = $cmd_name; $$ = $cmd_suffix; }
                 | cmd_name { $$ = new SimpleCommand($cmd_name); }
                 ;
cmd_name         : WORD                   /* Apply rule 7a */
                 ;
cmd_word         : WORD                   /* Apply rule 7b */
                 ;
cmd_prefix       :            io_redirect
                 | cmd_prefix io_redirect
                 |            ASSIGNMENT_WORD
                 | cmd_prefix ASSIGNMENT_WORD
                 ;
cmd_suffix       :            io_redirect
                 | cmd_suffix io_redirect
                 |            WORD { $$ = new SimpleCommand("", {}, {$WORD}, {}); }
                 | cmd_suffix WORD { $1->args.push_back($WORD); $$ = $1; }
                 ;
io_redirect      : '<'       filename
                 | '>'       filename
                 ;
filename         : WORD                      /* Apply rule 2 */
                 ;
newline_list     :              '\n'
                 | newline_list '\n'
                 ;
linebreak        : newline_list
                 | /* empty */
                 ;


%%

void yy::parser::error(const std::string &message)
{
    std::cerr << "Error: " << message << std::endl;
}
