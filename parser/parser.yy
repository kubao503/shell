%require "3.2"
%language "c++"
%define api.token.constructor

%code requires {
    #include <string>
    #include "ast.hh"
    #include "parser_output.hh"

    class Lexer;
}

%code top {
    #include "parser.hh"
    #include "lexer.hh"

    namespace yy {
        static parser::symbol_type yylex(Lexer &lexer) {
            return lexer.get_next_token();
        }
    }
}

%define api.value.type variant
%lex-param {Lexer &lexer}
%parse-param {Lexer &lexer} {ParserOutput &output}

%header

%token  <std::string> WORD ASSIGNMENT_WORD SUBSHELL

%nterm  <std::vector<Statement*>>   list
%nterm  <Statement*>                pipeline command simple_command
%nterm  <std::string>               cmd_name cmd_word filename
%nterm  <SimpleCommand*>            cmd_prefix cmd_suffix
%nterm  <std::vector<Redirection>>  redirect_list
%nterm  <Redirection>               io_redirect

%%

program          : linebreak complete_commands linebreak
                 | linebreak
                 ;
complete_commands: complete_commands newline_list complete_command
                 |                                complete_command
                 ;
complete_command : list ';'                         { output.interpret($list); }
                 | list                             { output.interpret($list); }
                 ;
list             : list ';' pipeline                { $1.push_back($pipeline); $$ = $1; }
                 |          pipeline                { $$ = std::vector<Statement*>{$pipeline}; }
                 ;
pipeline         :                        command
                 | pipeline '|' linebreak command   { $$ = new Pipeline($1, $command); }
                 ;
command          : simple_command
                 | ASSIGNMENT_WORD                  { $$ = new Assignment($ASSIGNMENT_WORD); }
                 | SUBSHELL                         { $$ = new SubShell($SUBSHELL); }
                 | SUBSHELL redirect_list           { $$ = new SubShell($SUBSHELL); }
                 ;
simple_command   : cmd_prefix cmd_word cmd_suffix   {
                                                        $1->redirections.insert($1->redirections.end(), $3->redirections.begin(), $3->redirections.end());
                                                        $1->args.insert($1->args.end(), $3->args.begin(), $3->args.end());
                                                        $1->name = $cmd_word;
                                                        $$ = $1;
                                                    }
                 | cmd_prefix cmd_word              { $cmd_prefix->name = $cmd_word; $$ = $cmd_prefix; }
                 | cmd_name cmd_suffix              { $cmd_suffix->name = $cmd_name; $$ = $cmd_suffix; }
                 | cmd_name                         { $$ = new SimpleCommand($cmd_name); }
                 ;
cmd_name         : WORD                   /* Apply rule 7a */
                 ;
cmd_word         : WORD                   /* Apply rule 7b */
                 ;
cmd_prefix       :            io_redirect           { $$ = new SimpleCommand("", {}, {}, {$io_redirect}); }
                 | cmd_prefix io_redirect           { $1->redirections.push_back($io_redirect); $$ = $1; }
                 ;
cmd_suffix       :            io_redirect           { $$ = new SimpleCommand("", {}, {}, {$io_redirect}); }
                 | cmd_suffix io_redirect           { $1->redirections.push_back($io_redirect); $$ = $1; }
                 |            WORD                  { $$ = new SimpleCommand("", {}, {$WORD}, {}); }
                 | cmd_suffix WORD                  { $1->args.push_back($WORD); $$ = $1; }
                 ;
redirect_list    :               io_redirect        { $$ = std::vector<Redirection>{$io_redirect}; }
                 | redirect_list io_redirect        { $1.push_back($io_redirect); }
                 ;
io_redirect      : '<'       filename               { $$ = Redirection{$filename, RedirectionType::IN}; }
                 | '>'       filename               { $$ = Redirection{$filename, RedirectionType::OUT}; }
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
