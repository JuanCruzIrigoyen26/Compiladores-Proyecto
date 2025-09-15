%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void yyerror(const char *s);
int yylex(void);
extern int yylineno;
%}

%debug

/* Tokens Literales, Identificadoes y Tipos */
%token ID
%token ENTERO
%token T_INT
%token T_BOOL
%token T_VOID
%token T_TRUE
%token T_FALSE

/* Tokens de Operadores Aritméticos */
%token T_SUMA
%token T_MULT
%token T_RESTA
%token T_DIVISION
%token T_MOD
%token T_ASIGNACION

/* Tokens de Operadores Lógicos */
%token T_NOT
%token UMINUS
%token T_MAYOR
%token T_MENOR
%token T_IGUAL
%token T_AND
%token T_OR

/* Tokens de Delimitadores */
%token T_PA
%token T_PC
%token T_LA
%token T_LC
%token T_COMA
%token T_PUNTOC

/* Tokens de Palabras Reservadas */
%token T_EXTERN
%token T_IF
%token T_THEN
%token T_ELSE
%token T_WHILE
%token T_RETURN
%token T_PROGRAM

%left T_OR
%left T_AND
%left T_IGUAL
%left T_MAYOR
%left T_MENOR
%left T_SUMA
%left T_RESTA
%left T_MULT
%left T_DIVISION
%left T_MOD
%right T_NOT
%right T_ASIGNACION
%right UMINUS

%%

/* Programa */
prog:
      T_PROGRAM T_LA decls T_LC   { printf("No hay errores\n"); }
    ;

/* Declaraciones (Chequeo de vacio) */
decls:
      /* vacio */
    | decls decl
    ;

/* Declaración */
decl:
      tipo_decl ID decl_var_or_metodo
    ;

/* Declaración de variable o método */
decl_var_or_metodo:
      T_ASIGNACION expr T_PUNTOC
    | T_PA parametros T_PC bloque
    | T_PA parametros T_PC T_EXTERN T_PUNTOC
    ;

/* Parámetros (Chequeo de vacio) */
parametros:
      /* vacio */
    | lista_param
    ;

/* Bloque de Sentencias */
bloque:
      T_LA decls_sentencias T_LC
    ;

/* Sentencias (chequeo de vacio) */
decls_sentencias:
      /* vacio */
    | decls_sentencias decl_or_sentencia
    ;

/* Declaracion de variable o sentencia */
decl_or_sentencia:
      tipo ID T_ASIGNACION expr T_PUNTOC
    | sentencia
    ;

/* Sentencia */
sentencia:
      asignacion T_PUNTOC
    | llamada_func T_PUNTOC
    | T_IF T_PA expr T_PC T_THEN bloque
    | T_IF T_PA expr T_PC T_THEN bloque T_ELSE bloque
    | T_WHILE T_PA expr T_PC bloque
    | T_RETURN expr T_PUNTOC
    | T_RETURN T_PUNTOC
    | T_PUNTOC
    ;

/* Llamada a método/función */
llamada_func:
      ID T_PA expresiones T_PC
    ;

/* Asignación */
asignacion:
      ID T_ASIGNACION expr
    ;

/* Expresiones (Chequeo de vacio) */
expresiones:
      /* vacio */
    | lista_expr
    ;

/* Lista de expresiones sucesiva o unitario */
lista_expr:
      expr
    | lista_expr T_COMA expr
    ;

/* Lista de parámetros o unitario */
lista_param:
      tipo ID
    | lista_param T_COMA tipo ID
    ;

/* Tipo para declaraciones */
tipo_decl:
      T_INT
    | T_BOOL
    | T_VOID
    ;

/* Tipo para parámetros */
tipo:
      T_INT
    | T_BOOL
    ;

/* Expresion */
expr:
      valor
    | ID T_PA expresiones T_PC
    | expr T_SUMA expr
    | expr T_RESTA expr
    | expr T_MULT expr
    | expr T_DIVISION expr
    | expr T_MOD expr
    | expr T_MAYOR expr
    | expr T_MENOR expr
    | expr T_IGUAL expr
    | expr T_OR expr
    | expr T_AND expr
    | T_NOT expr
    | T_RESTA expr %prec UMINUS
    | T_PA expr T_PC
    ;

/* Valor de variable */
valor:
      ID
    | ENTERO
    | T_TRUE
    | T_FALSE
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error sintáctico en línea %d: %s\n", yylineno, s);
}

int main(int argc, char **argv) {
    if(yyparse() == 0){
      return 0;
    }else{
      return 1;
    }
}

