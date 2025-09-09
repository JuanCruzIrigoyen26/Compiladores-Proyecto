%{
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "baison.tab.h"
%}

%option noyywrap
%option yylineno  /* para que bison pueda usar el número de línea */


entero [0-9]
letra [a-zA-Z]


%%


/* Literales e Identificadores */
{entero}+   { /*yylval.valor.i = atol(yytext);*/
              return ENTERO; }

{letra}({letra}|{entero})* {
              /*yylval.valor.s = strdup(yytext);*/
              return ID;
}

/* Palabras Reservadas */
"integer"   { return T_INT; } 
"bool"      { return T_BOOL; } 
"void"      { return T_VOID; } 
"return"    { return T_RETURN; } 
"true"      { /*yylval.valor.b = 1;*/
              return T_TRUE; } 
"false"     { /*yylval.valor.b = 0;*/ 
              return T_FALSE; } 
"extern"    { return T_EXTERN; }
"program"   { return T_PROGRAM; } 
"else"      { return T_ELSE; } 
"then"      { return T_THEN; } 
"if"        { return T_IF; } 
"while"     { return T_WHILE; }

/* Operadores Aritméticos */
"+"         { /*yylval.valor.op = '+';*/
              return T_SUMA; }
"*"         { /*yylval.valor.op = '*';*/ 
              return T_MULT; }
"-"         { /*yylval.valor.op = '-';*/
              return T_RESTA; }
"/"         { /*yylval.valor.op = '/';*/
              return T_DIVISION; }
"%"         { /*yylval.valor.op = '%';*/
              return T_MOD; }

/* Operadores Relacionales */
">"         { /*yylval.valor.op = '>';*/
			  return T_MAYOR; }
"<"         { /*yylval.valor.op = '<';*/
			  return T_MENOR; }
"=="        { return T_IGUAL; }

/* Operadores Lógicos */
"&&"        { return T_AND; }
"||"        { return T_OR; }

/* Asignación */
"="         { /*yylval.valor.op = '=';*/
			  return T_ASIGNACION; }

/* Delimitadores */
";"         { return T_PUNTOC; }
"("         { return T_PA; }
")"         { return T_PC; }
"{"         { return T_LA; }
"}"         { return T_LC; }

/* Comentarios */
/* Comentario de línea */
"//".*          { /* ignorar hasta el final de la línea */ }

/* Comentario de bloque */
"/*"([^*]|\*+[^*/])*\*+"/"   { /* ignorar bloque */ }

/* Espacios en blanco */
[ \t\r\n]+      { /* ignorar espacios y saltos de línea */ }

/* Caracteres inválidos */
.               { fprintf(stderr, "Caracter inválido '%s' en línea %d\n", yytext, yylineno); }

%%
