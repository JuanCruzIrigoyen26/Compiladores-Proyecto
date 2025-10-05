#ifndef CODIGOINTERMEDIO_H
#define CODIGOINTERMEDIO_H

#include "ast.h"

// Tipos de Instrucciones del codigo intermedio
typedef enum {
    INSTR_ADD,
    INSTR_SUB,
    INSTR_MUL,
    INSTR_DIV,
    INSTR_MOD,
    INSTR_LT,
    INSTR_GT,
    INSTR_EQ,
    INSTR_AND,
    INSTR_OR,
    INSTR_NOT,
    INSTR_ASSIGN,
    INSTR_IF_TRUE,
    INSTR_IF_FALSE,
    INSTR_GOTO,
    INSTR_LABEL,
    INSTR_FUNC_BEGIN,
    INSTR_FUNC_END,
    INSTR_PARAM,
    INSTR_CALL,
    INSTR_RETVAL,
    INSTR_RETURN,
    INSTR_VAR_GLOBAL,
    INSTR_VAR_LOCAL,
    INSTR_UNKNOWN
} TipoInstr;

// Estructura para Instruccion
typedef struct Instr {
    TipoInstr tipo;
    char* arg1;
    char* arg2;
    char* res;
    char* label;
    struct Instr* next;
} Instr;

// Estructura para el codigo intermedio
typedef struct {
    Instr* head;
    Instr* tail;
    int tempCount;
    int labelCount;
} CodigoIntermedio;

// Funciones del generador de codigo intermedio
CodigoIntermedio* crearGenerador();
void liberarGenerador(CodigoIntermedio* generador);
void generarCodigoIntermedio(CodigoIntermedio* genenerador, Nodo* raiz);
void imprimirCodigoIntermedio(CodigoIntermedio* genenerador);

#endif
