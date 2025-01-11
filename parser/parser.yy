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

%nterm  <std::vector<std::unique_ptr<Statement>>>   list
%nterm  <std::unique_ptr<Statement>>                pipeline command simple_command
%nterm  <std::string>                               cmd_name cmd_word filename
%nterm  <std::unique_ptr<SimpleCommand>>            cmd_prefix cmd_suffix
%nterm  <std::vector<Redirection>>                  redirect_list
%nterm  <Redirection>                               io_redirect

%%

program          : linebreak complete_commands linebreak
                 | linebreak
                 ;
complete_commands: complete_commands newline_list complete_command
                 |                                complete_command
                 ;
complete_command : list ';'                         { output.interpret(std::move($list)); }
                 | list                             { output.interpret(std::move($list)); }
                 ;
list             : list ';' pipeline                { $1.push_back(std::move($pipeline)); $$ = std::move($1); }
                 |          pipeline                { $$ = std::vector<std::unique_ptr<Statement>>(); $$.push_back(std::move($pipeline)); }
                 ;
pipeline         :                        command   { $$ = std::move($1); }
                 | pipeline '|' linebreak command   { $$ = std::make_unique<Pipeline>(std::move($1), std::move($command)); }
                 ;
command          : simple_command                   { $$ = std::move($1); }
                 | ASSIGNMENT_WORD                  { $$ = std::make_unique<Assignment>($ASSIGNMENT_WORD); }
                 | SUBSHELL                         { $$ = std::make_unique<Subshell>($SUBSHELL); }
                 | SUBSHELL redirect_list           { $$ = std::make_unique<Subshell>($SUBSHELL); }
                 ;
simple_command   : cmd_prefix cmd_word cmd_suffix   {
                                                        $1->redirections.insert($1->redirections.end(), $3->redirections.begin(), $3->redirections.end());
                                                        $1->args.insert($1->args.end(), $3->args.begin(), $3->args.end());
                                                        $1->name = $cmd_word;
                                                        $$ = std::move($1);
                                                    }
                 | cmd_prefix cmd_word              { $cmd_prefix->name = $cmd_word; $$ = std::move($cmd_prefix); }
                 | cmd_name cmd_suffix              { $cmd_suffix->name = $cmd_name; $$ = std::move($cmd_suffix); }
                 | cmd_name                         { $$ = std::make_unique<SimpleCommand>($cmd_name); }
                 ;
cmd_name         : WORD                   /* Apply rule 7a */
                 ;
cmd_word         : WORD                   /* Apply rule 7b */
                 ;
cmd_prefix       :            io_redirect           { $$ = std::make_unique<SimpleCommand>(); $$->redirections = {$io_redirect}; }
                 | cmd_prefix io_redirect           { $1->redirections.push_back($io_redirect); $$ = std::move($1); }
                 ;
cmd_suffix       :            io_redirect           { $$ = std::make_unique<SimpleCommand>(); $$->redirections = {$io_redirect}; }
                 | cmd_suffix io_redirect           { $1->redirections.push_back($io_redirect); $$ = std::move($1); }
                 |            WORD                  { $$ = std::make_unique<SimpleCommand>(); $$->args = {$WORD}; }
                 | cmd_suffix WORD                  { $1->args.push_back($WORD); $$ = std::move($1); }
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
