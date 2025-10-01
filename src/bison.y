%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "ts.h"

Nodo* raiz = NULL;

void yyerror(const char *s);
int yylex(void);
extern int yylineno;
%}

%union {
    Nodo* nodo;      
    AstValor valor;
}

%debug

/* Tokens Literales, Identificadoes y Tipos */
%token <valor> ID
%token <valor> ENTERO
%token <valor> T_INT
%token <valor> T_BOOL
%token <valor> T_VOID
%token <valor> T_TRUE
%token <valor> T_FALSE

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
%token <valor> T_RETURN
%token T_PROGRAM

/* Declaramos qué reglas devuelven nodos */
%type <nodo> prog decls decl decl_func perfil_func decl_func_extern decl_var_global parametros bloque bloque_cuerpo
%type <nodo> decls_sentencias decl_or_sentencia sentencia llamada_func
%type <nodo> asignacion expresiones lista_expr lista_param
%type <nodo> tipo_decl tipo expr valor

/* Precedencias (incluye dangling-else) */
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
%nonassoc T_THEN
%nonassoc T_ELSE

%%

/* Programa */
prog:
      T_PROGRAM T_LA decls T_LC
      {
          raiz = nodo_binario(AST_PROG, (AstValor){0}, $3, NULL);
          $$ = raiz;
      }
;

/* Declaraciones (Chequeo de vacio) */
decls:
      /* vacio */ { $$ = NULL; }
    | decl decls  { $$ = nodo_binario(AST_DECLS, (AstValor){0}, $1, $2); }
;

/* Tipos de declaración */
decl:
      decl_func
    | decl_func_extern
    | decl_var_global
;

/* Definición de función */
decl_func:
      perfil_func bloque_cuerpo
      {
        $$ = nodo_ternario(AST_DECL_FUNC, *($1->v), $1->hi, $2, NULL);
      }
;

/* Perfil de la función */
perfil_func:
      tipo_decl ID T_PA parametros T_PC
      {
          AstValor vFunc = (AstValor){0};
          vFunc.s = strdup($2.s);
          vFunc.tipoDef = $1->v->tipoDef;
          vFunc.esFuncion = 1;
          vFunc.linea = yylineno;
          insertarSimbolo(&vFunc);
          Nodo* n = nodo_ternario(AST_DECL_FUNC, vFunc, $4, NULL, NULL);
          abrirNivel();
          insertarParametros($4);
          $$ = n;          
      }
;

/* Declaración extern */
decl_func_extern:
      tipo_decl ID T_PA parametros T_PC T_EXTERN T_PUNTOC
      {
          AstValor vFunc = (AstValor){0};
          vFunc.s = strdup($2.s);
          vFunc.tipoDef = $1->v->tipoDef;
          vFunc.esFuncion = 1;
          vFunc.linea = yylineno;
          insertarSimbolo(&vFunc);
          $$ = nodo_ternario(AST_DECL_FUNC, vFunc, $4, NULL, NULL);
      }
;

/* Variable global */
decl_var_global:
      tipo_decl ID T_ASIGNACION expr T_PUNTOC
      {
          AstValor v = (AstValor){0};
          v.s = $2.s;
          v.tipoDef = $1->v->tipoDef;
          v.linea = yylineno;
          if ($4 && $4->tipo == AST_INT) {
              v.i = $4->v->i;
          } else if ($4 && $4->tipo == AST_BOOL) {
              v.b = $4->v->b;
          }
          if (existeEnNivelActual(v.s)) {
            fprintf(stderr, "Error semántico en línea %d: Identificador declarado dos veces en el mismo bloque\n", yylineno);
          } else {
            insertarSimbolo(&v);
          }
          $$ = nodo_binario(AST_DECL_VAR, v, $1, $4);
      }
;


/* Parámetros (Chequeo de vacio) */
parametros:
      /* vacio */ { $$ = NULL; }
    | lista_param { $$ = $1; }
;

/* Bloque normal (if, while) */
bloque:
      T_LA { abrirNivel(); } decls_sentencias T_LC
      {
          Nodo* declsNodo = $3 ? nodo_binario(AST_DECLS, (AstValor){0}, $3, NULL) : NULL;
          $$ = nodo_binario(AST_BLOQUE, (AstValor){0}, declsNodo, NULL);
          cerrarNivel();
      }
;

/* Bloque de cuerpo de función */
bloque_cuerpo:
      T_LA decls_sentencias T_LC
      {
          Nodo* declsNodo = $2 ? nodo_binario(AST_DECLS, (AstValor){0}, $2, NULL) : NULL;
          $$ = nodo_binario(AST_BLOQUE, (AstValor){0}, declsNodo, NULL);
          cerrarNivel();
      }
;

/* Sentencias (chequeo de vacio) */
decls_sentencias:
      /* vacio */ { $$ = NULL; }
    | decls_sentencias decl_or_sentencia { $$ = nodo_binario(AST_STMTS, (AstValor){0}, $1, $2); }
;

/* Declaracion de variable local o sentencia */
decl_or_sentencia:
      tipo ID T_ASIGNACION expr T_PUNTOC
      {
          AstValor v = (AstValor){0};
          v.s = $2.s;
          v.tipoDef = $1->v->tipoDef;
          v.linea = yylineno;
          if ($4 && $4->tipo == AST_INT) {
              v.i = $4->v->i;
          } else if ($4 && $4->tipo == AST_BOOL) {
              v.b = $4->v->b;
          }
          if (existeEnNivelActual(v.s)) {
            fprintf(stderr, "Error semántico en línea %d: Identificador declarado dos veces en el mismo bloque\n", yylineno);
          } else {
            insertarSimbolo(&v);
          }
          $$ = nodo_binario(AST_DECL_VAR, v, $1, $4);
      }
    | sentencia { $$ = $1; }
;

/* Sentencia */
sentencia:
      asignacion T_PUNTOC { $$ = $1; }
    | llamada_func T_PUNTOC { $$ = $1; }
    | T_IF T_PA expr T_PC T_THEN bloque %prec T_THEN { $$ = nodo_binario(AST_IF, (AstValor){0}, $3, $6); }
    | T_IF T_PA expr T_PC T_THEN bloque T_ELSE bloque { $$ = nodo_ternario(AST_IF, (AstValor){0}, $3, $6, $8); }
    | T_WHILE T_PA expr T_PC bloque { $$ = nodo_binario(AST_WHILE, (AstValor){0}, $3, $5); }
    | T_RETURN expr T_PUNTOC { $$ = nodo_binario(AST_RETURN, (AstValor){0}, $2, NULL); }
    | T_RETURN T_PUNTOC { $$ = nodo_hoja(AST_RETURN, (AstValor){0}); }
    | T_PUNTOC { $$ = NULL; }
    | bloque   { $$ = $1; }
;

/* Llamada a método/función */
llamada_func:
    ID T_PA expresiones T_PC
    {
        $1.linea = yylineno;
        $$ = nodo_binario(AST_LLAMADA, (AstValor){0}, nodo_hoja(AST_ID, $1), $3);
    }
;

/* Asignación */
asignacion:
      ID T_ASIGNACION expr
      { 
        $1.linea = yylineno;
        $$ = nodo_binario(AST_ASIGNACION, (AstValor){0}, nodo_hoja(AST_ID, $1), $3);
      }
;

/* Expresiones (Chequeo de vacio) */
expresiones:
      /* vacio */   { $$ = NULL; }
    | lista_expr    { $$ = $1; }
;

/* Lista de expresiones sucesiva o unitario */
lista_expr:
      expr  { $$ = $1; }
    | lista_expr T_COMA expr { $$ = nodo_binario(AST_SEQ_EXPR, (AstValor){0}, $1, $3); }
;

/* Lista de parámetros o unitario */
lista_param:
      tipo ID
      {
          AstValor v = (AstValor){0};
          v.s = $2.s;
          v.tipoDef = $1->v->tipoDef;
          v.linea = yylineno;
          $$ = nodo_hoja(AST_PARAM, v);
      }
    | lista_param T_COMA tipo ID
      {
          AstValor v = (AstValor){0};
          v.s = $4.s;
          v.tipoDef = $3->v->tipoDef;
          v.linea = yylineno;
          $$ = nodo_binario(AST_PARAMS, (AstValor){0}, $1, nodo_hoja(AST_PARAM, v));
      }
;

/* Tipo para declaraciones */
tipo_decl:
      T_INT   { AstValor v = (AstValor){0}; v.tipoDef = INT; $$ = nodo_hoja(AST_INT, v); }
    | T_BOOL  { AstValor v = (AstValor){0}; v.tipoDef = BOOL; $$ = nodo_hoja(AST_BOOL, v); }
    | T_VOID  { AstValor v = (AstValor){0}; v.tipoDef = VOID; $$ = nodo_hoja(AST_VOID, v); }
;

/* Tipo para parámetros */
tipo:
      T_INT   { AstValor v = (AstValor){0}; v.tipoDef = INT; $$ = nodo_hoja(AST_INT, v); }
    | T_BOOL  { AstValor v = (AstValor){0}; v.tipoDef = BOOL; $$ = nodo_hoja(AST_BOOL, v); }
;

/* Expresion */
expr:
      valor { $$ = $1; }
    | ID T_PA expresiones T_PC { $$ = nodo_binario(AST_LLAMADA, (AstValor){0}, nodo_hoja(AST_ID, $1), $3); }
    | expr T_SUMA expr { $$ = nodo_binario(AST_OP, (AstValor){.op=OP_SUMA}, $1, $3); }
    | expr T_RESTA expr { $$ = nodo_binario(AST_OP, (AstValor){.op=OP_RESTA}, $1, $3); }
    | expr T_MULT expr { $$ = nodo_binario(AST_OP, (AstValor){.op=OP_MULT}, $1, $3); }
    | expr T_DIVISION expr { $$ = nodo_binario(AST_OP, (AstValor){.op=OP_DIV}, $1, $3); }
    | expr T_MOD expr { $$ = nodo_binario(AST_OP, (AstValor){.op=OP_MOD}, $1, $3); }
    | expr T_MAYOR expr { $$ = nodo_binario(AST_OP, (AstValor){.op=OP_MAYOR}, $1, $3); }
    | expr T_MENOR expr { $$ = nodo_binario(AST_OP, (AstValor){.op=OP_MENOR}, $1, $3); }
    | expr T_IGUAL expr { $$ = nodo_binario(AST_OP, (AstValor){.op=OP_IGUAL}, $1, $3); }
    | expr T_AND expr { $$ = nodo_binario(AST_OP, (AstValor){.op=OP_AND}, $1, $3); }
    | expr T_OR expr { $$ = nodo_binario(AST_OP, (AstValor){.op=OP_OR}, $1, $3); }
    | T_NOT expr { $$ = nodo_binario(AST_OP, (AstValor){.op=OP_NOT}, $2, NULL); }
    | T_RESTA expr %prec UMINUS { $$ = nodo_binario(AST_OP, (AstValor){.op=OP_RESTA}, nodo_hoja(AST_INT, (AstValor){.i=0, .tipoDef=INT}), $2); }
    | T_PA expr T_PC { $$ = $2; }
;

/* Valor de variable */
valor:
      ID { $1.linea = yylineno; $$ = nodo_hoja(AST_ID, $1); }
    | ENTERO { $1.tipoDef = INT; $1.linea = yylineno; $$ = nodo_hoja(AST_INT, $1); }
    | T_TRUE { $1.tipoDef = BOOL;  $1.linea = yylineno; $$ = nodo_hoja(AST_BOOL, $1); }
    | T_FALSE { $1.tipoDef = BOOL; $1.linea = yylineno; $$ = nodo_hoja(AST_BOOL, $1); }
;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error sintáctico en línea %d: %s\n", yylineno, s);
}
