%option noyywrap

%{
#include <stdio.h>
%}

%%

"edike ay mama"        { fprintf(yyout, "<START_PROGRAM>\n"); }
"mama chole ja"        { fprintf(yyout, "<END_PROGRAM>\n"); }

"mama eita hoilo"      { fprintf(yyout, "<VAR_DECL>\n"); }

"bol mama"             { fprintf(yyout, "<OUTPUT>\n"); }
"shon mamaa"           { fprintf(yyout, "<INPUT>\n"); }

"jodi mama"            { fprintf(yyout, "<IF>\n"); }
"nahole mama"          { fprintf(yyout, "<ELSE>\n"); }

"mama jotokkhon"       { fprintf(yyout, "<WHILE>\n"); }

"=="                   { fprintf(yyout, "<EQ_OP>\n"); }
">="                   { fprintf(yyout, "<GE_OP>\n"); }
"<="                   { fprintf(yyout, "<LE_OP>\n"); }
">"                    { fprintf(yyout, "<GT_OP>\n"); }
"<"                    { fprintf(yyout, "<LT_OP>\n"); }
"="                    { fprintf(yyout, "<ASSIGN_OP>\n"); }

"+"                    { fprintf(yyout, "<PLUS>\n"); }
"-"                    { fprintf(yyout, "<MINUS>\n"); }

"("                    { fprintf(yyout, "<LPAREN>\n"); }
")"                    { fprintf(yyout, "<RPAREN>\n"); }
"{"                    { fprintf(yyout, "<LBRACE>\n"); }
"}"                    { fprintf(yyout, "<RBRACE>\n"); }

[0-9]+                 { fprintf(yyout, "<NUMBER : %s>\n", yytext); }
\"[^\"]*\"             { fprintf(yyout, "<STRING : %s>\n", yytext); }
\'[a-zA-Z]\'           { fprintf(yyout, "<CHAR : %s>\n", yytext); }

[a-zA-Z_][a-zA-Z0-9_]* { fprintf(yyout, "<IDENTIFIER : %s>\n", yytext); }

[ \t\n]+               { /* ignore whitespace */ }

.                      { fprintf(yyout, "<UNKNOWN : %s>\n", yytext); }

%%

int main(void)
{
    yyin  = fopen("input.txt", "r");
    yyout = fopen("output.txt", "w");

    if (!yyin || !yyout) {
        printf("File error!\n");
        return 1;
    }

    yylex();

    fclose(yyin);
    fclose(yyout);

    return 0;
}
