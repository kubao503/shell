%require "3.2"
%language "c++"

%code requires {
    #include <string>
    #include "FooLexer.hh"
}

%define api.value.type {std::string}

%parse-param {FooLexer &lexer}

%header

%code {
    #define yylex lexer.yylex
}

%token  WORD
%token  ASSIGNMENT_WORD
%token  NEWLINE

%%

program          : linebreak complete_commands linebreak
                 | linebreak
                 ;
complete_commands: complete_commands newline_list complete_command  { std::cout << "Parsed program\n"; }
                 |                                complete_command  { std::cout << "Parsed program\n"; }
                 ;
complete_command : list ';'
                 | list
                 ;
list             : list ';' pipeline
                 |          pipeline
                 ;
pipeline         :                        command
                 | pipeline '|' linebreak command
                 ;
command          : simple_command
                 | subshell
                 ;
subshell         : '(' compound_list ')'
                 ;
compound_list    : linebreak term
                 | linebreak term separator
                 ;
term             : term separator pipeline
                 |                pipeline
                 ;
simple_command   : cmd_prefix cmd_word cmd_suffix
                 | cmd_prefix cmd_word
                 | cmd_prefix
                 | cmd_name cmd_suffix
                 | cmd_name
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
                 |            WORD
                 | cmd_suffix WORD
                 ;
redirect_list    :               io_redirect
                 | redirect_list io_redirect
                 ;
io_redirect      : '<'       filename
                 | '>'       filename
                 ;
filename         : WORD                      /* Apply rule 2 */
                 ;
newline_list     :              NEWLINE
                 | newline_list NEWLINE
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
