%{
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "bison.tab.h"

extern int yylineno;
%}

%option noyywrap
%option yylineno

entero [0-9]
letra [a-zA-Z_]

%%

"integer"   { return T_INT; } 
"bool"      { return T_BOOL; } 
"void"      { return T_VOID; } 
"return"    { return T_RETURN; } 
"true"      { return T_TRUE; } 
"false"     { return T_FALSE; } 
"extern"    { return T_EXTERN; }
"program"   { return T_PROGRAM; } 
"else"      { return T_ELSE; } 
"then"      { return T_THEN; } 
"if"        { return T_IF; } 
"while"     { return T_WHILE; }

{entero}+   { return ENTERO; }
{letra}({letra}|{entero})* { return ID; }

"=="        { return T_IGUAL; }
"+"         { return T_SUMA; }
"*"         { return T_MULT; }
"-"         { return T_RESTA; }
"/"         { return T_DIVISION; }
"%"         { return T_MOD; }

">"         { return T_MAYOR; }
"<"         { return T_MENOR; }

"&&"        { return T_AND; }
"||"        { return T_OR; }
"!"         { return T_NOT; }

"="         { return T_ASIGNACION; }

";"         { return T_PUNTOC; }
","         { return T_COMA; }
"("         { return T_PA; }
")"         { return T_PC; }
"{"         { return T_LA; }
"}"         { return T_LC; }

"//".*              { /* ignorar comentario de línea */ }
"/*"([^*]|\n)*"*/"  { /* ignorar comentario de bloque */ }

[ \t\r\n]+          { /* ignorar */ }

.   { fprintf(stderr, "Caracter inválido '%s' en línea %d\n", yytext, yylineno); }

%%
