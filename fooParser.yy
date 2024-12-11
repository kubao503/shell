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
}

%define api.value.type variant
%lex-param {FooLexer &lexer}
%parse-param {FooLexer &lexer}

%header

%token  <std::string> WORD
%token  <std::string> ASSIGNMENT_WORD
%token  NEWLINE
%type   <Program*> simple_command

%%

program          : linebreak complete_commands linebreak
                 | linebreak
                 ;
complete_commands: complete_commands newline_list complete_command
                 |                                complete_command
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
simple_command   : WORD cmd_suffix              {
                                                        auto program = new Program{$WORD, {}, {}};
                                                        std::cout << program->name << '\n';
                                                        $$ = program;
                                                    }
                 | cmd_prefix                       {
                                                        auto program = new Program{"asgn", {}, {}};
                                                        std::cout << program->name << '\n';
                                                        $$ = program;
                                                    }
                 | WORD                         {
                                                        auto program = new Program{$WORD, {}, {}};
                                                        std::cout << program->name << '\n';
                                                        $$ = program;
                                                    }
                 ;
cmd_prefix       : ASSIGNMENT_WORD
                 | /* empty */
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
