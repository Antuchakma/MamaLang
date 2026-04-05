%option yylineno
%{
#include "parser.tab.h"
#include <string.h>
#include <stdlib.h>
%}

%%
"edike_ay_mama"              { return START; }
"mama_chole_ja"              { return END; }
"mama_eita_hoilo"            { return DECL; }
"bol_mama"                   { return PRINT; }
"shon_mama"                  { return INPUT; }
"jodi_mama"                  { return IF; }
"nahole_mama"                { return ELSE; }
"mama_jotokkhon"             { return WHILE; }
"jotokkhon"                  { return FOR; }
"mama_kaj"                   { return FUNC; }
"ferot_de_mama"              { return RETURN; }

"=="                         { return EQ; }
"!="                         { return NEQ; }
"<="                         { return LEQ; }
">="                         { return GEQ; }

[0-9]+\.[0-9]+                { yylval.str = strdup(yytext); return FLOATLIT; }
[0-9]+                        { yylval.str = strdup(yytext); return NUMBER; }
\'([^\\\n]|\\.)\'             { yylval.str = strdup(yytext); return CHARLIT; }
\"([^\\\n]|\\.)*\"            { yylval.str = strdup(yytext); return STRING; }
[a-zA-Z_][a-zA-Z0-9_]*       { yylval.str = strdup(yytext); return IDENT; }

[ \t\r]+                     { /* ignore spaces */ }
\n                            { return NEWLINE; }

"//".*                       { /* single line comment */ }

[{}(),;+\-*/%<>=]            { return yytext[0]; }

.                             {
                                  fprintf(stderr, "[Line %d] Invalid token: %s\n", yylineno, yytext);
                                  return yytext[0];
                              }
%%

int yywrap(void) {
    return 1;
}
