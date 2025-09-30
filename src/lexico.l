%{
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ast.h"
#include "bison.tab.h"

extern int yylineno;
int contador_comentario = 0;

%}

%option noyywrap
%option yylineno

%x COMENTARIO

entero [0-9]
letra [a-zA-Z_]

%%

"integer"   { return T_INT; }       /* Palabras Reservadas */
"bool"      { return T_BOOL; } 
"void"      { return T_VOID; } 
"return"    { return T_RETURN; } 
"true"      { yylval.valor.b = 1;
              return T_TRUE; } 
"false"     { yylval.valor.b = 0;
              return T_FALSE; } 
"extern"    { return T_EXTERN; }
"program"   { return T_PROGRAM; } 
"else"      { return T_ELSE; } 
"then"      { return T_THEN; } 
"if"        { return T_IF; } 
"while"     { return T_WHILE; }

{entero}+   { yylval.valor.i = atol(yytext);    /* Literales e Identificadores */
              return ENTERO; }      
{letra}({letra}|{entero})* { yylval.valor.s = strdup(yytext);
                             return ID; }

"+"         { yylval.valor.op = OP_SUMA;      /* Operadores Aritméticos */
              return T_SUMA; }      
"*"         { yylval.valor.op = OP_MULT;
              return T_MULT; }
"-"         { yylval.valor.op = OP_RESTA;
              return T_RESTA; }
"/"         { yylval.valor.op = OP_DIV;
              return T_DIVISION; }
"%"         { yylval.valor.op = OP_MOD;
              return T_MOD; }

">"         { yylval.valor.op = OP_MAYOR;
              return T_MAYOR; }     /* Operadores Aritméticos */
"<"         { yylval.valor.op = OP_MENOR;
              return T_MENOR; }
"=="        { yylval.valor.op = OP_IGUAL;
              return T_IGUAL; }

"&&"        { yylval.valor.op = OP_AND;       /* Operadores Lógicos */
              return T_AND; }       
"||"        { yylval.valor.op = OP_OR;
              return T_OR; }
"!"         { yylval.valor.op = OP_NOT;
              return T_NOT; }

"="         { yylval.valor.op = OP_ASIG;       /* Asignación */
              return T_ASIGNACION; }    

";"         { return T_PUNTOC; }    /* Delimitadores */
","         { return T_COMA; }
"("         { return T_PA; }
")"         { return T_PC; }
"{"         { return T_LA; }
"}"         { return T_LC; }

"//".*              { /* ignorar comentario de línea */ }   /* Comentario de Línea */

"/*"                { contador_comentario = 1; BEGIN(COMENTARIO); } /* Comentario de Bloque */

<COMENTARIO>"/*"    { contador_comentario++; }
<COMENTARIO>"*/"    { if (--contador_comentario == 0) BEGIN(INITIAL); }
<COMENTARIO>\n      { yylineno++; }
<COMENTARIO>.       { /* ignorar */ }

[ \t\r\n]+          { /* ignorar espacios y saltos de línea */ }  /* Espacios en blanco */

.   { fprintf(stderr, "Caracter inválido '%s' en línea %d\n", yytext, yylineno); }  /* Caracteres Inválidos */

%%
