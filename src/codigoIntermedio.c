#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codigoIntermedio.h"

static void generarSentencia(CodigoIntermedio* generador, Nodo* n);

// Crea el generador de codigo intermedio
CodigoIntermedio* crearGenerador() {
    CodigoIntermedio* generador = malloc(sizeof(CodigoIntermedio));
    generador->head = generador->tail = NULL;
    generador->tempCount = 0;
    generador->labelCount = 0;
    return generador;
}

// Funcion principal de generacion del codigo intermedio
void generarCodigoIntermedio(CodigoIntermedio* generador, Nodo* raiz) {
    generarSentencia(generador, raiz);
}

// Impresion del codigo intermedio
void imprimirCodigoIntermedio(CodigoIntermedio* generador) {
    // Implementacion pendiente
}

static void generarFuncion(CodigoIntermedio* generador, Nodo* n) {
    if (!n || n->tipo != AST_DECL_FUNC) return;

    const char* nombre = n->v->s;
    crearInstr(generador, INSTR_FUNC_BEGIN, nombre, NULL, NULL, NULL);

    generarSentencia(generador, n->hd);

    crearInstr(generador, INSTR_FUNC_END, nombre, NULL, NULL, NULL);
}

// Genera el codigo intermedio para declaraciones de variables globales o formales
static void generarDeclaracion(CodigoIntermedio* generador, Nodo* n, int esGlobal) {
    if (!n || n->tipo != AST_DECL_VAR) return;

    const char* tipoStr;
    if (n->v->tipoDef == INT) {
        tipoStr = "int";
    } else if (n->v->tipoDef == BOOL) {
        tipoStr = "bool";
    } else {
        fprintf(stderr, "Error interno: tipo de variable desconocido\n");
        tipoStr = "int";
    }

    if (esGlobal)
        crearInstr(generador, INSTR_VAR_GLOBAL, tipoStr, NULL, n->v->s, NULL);
    else
        crearInstr(generador, INSTR_VAR_LOCAL, tipoStr, NULL, n->v->s, NULL);

    if (n->hd) {
        char* expr = generarExpr(generador, n->hd);
        crearInstr(generador, INSTR_ASSIGN, expr, NULL, n->v->s, NULL);
        free(expr);
    }
}

// Genera el codigo intermedio para sentencias
static void generarSentencia(CodigoIntermedio* generador, Nodo* n) {
    if (!n) return;

    switch (n->tipo) {
        case AST_DECL_FUNC:
            generarFuncion(generador, n);
            break;

        case AST_DECL_VAR:
            generarDeclaracion(generador, n, 1);
            break;

        case AST_ASIGNACION: {
            char* aux = generarExpr(generador, n->hd);
            crearInstr(generador, INSTR_ASSIGN, aux, NULL, n->hi->v->s, NULL);
            free(aux);
            break;
        }

        case AST_STMTS:
        case AST_DECLS:
        case AST_BLOQUE:
            generarSentencia(generador, n->hi);
            generarSentencia(generador, n->hd);
            break;

        case AST_IF: {
            char* cond = generarExpr(generador, n->hi);
            char* charElse = nuevoLabel(generador);
            char* finIf = nuevoLabel(generador);

            crearInstr(generador, INSTR_IF_FALSE, cond, NULL, charElse, NULL);
            generarSentencia(generador, n->hd);
            crearInstr(generador, INSTR_GOTO, NULL, NULL, finIf, NULL);
            crearInstr(generador, INSTR_LABEL, NULL, NULL, NULL, charElse);
            if (n->extra) generarSentencia(generador, n->extra);
            crearInstr(generador, INSTR_LABEL, NULL, NULL, NULL, finIf);

            free(cond);
            free(charElse);
            free(finIf);
            break;
        }

        case AST_WHILE: {
            char* comienzoWhile = nuevoLabel(generador);
            char* finWhile = nuevoLabel(generador);

            crearInstr(generador, INSTR_LABEL, NULL, NULL, NULL, comienzoWhile);
            char* cond = generarExpr(generador, n->hi);
            crearInstr(generador, INSTR_IF_FALSE, cond, NULL, finWhile, NULL);
            generarSentencia(generador, n->hd);
            crearInstr(generador, INSTR_GOTO, NULL, NULL, comienzoWhile, NULL);
            crearInstr(generador, INSTR_LABEL, NULL, NULL, NULL, finWhile);

            free(comienzoWhile);
            free(finWhile);
            free(cond);
            break;
        }

        case AST_RETURN: {
            if (n->hi) {
                char* valor = generarExpr(generador, n->hi);
                crearInstr(generador, INSTR_RETURN, valor, NULL, NULL, NULL);
                free(valor);
            } else {
                crearInstr(generador, INSTR_RETURN, NULL, NULL, NULL, NULL);
            }
            break;
        }

        default:
            generarSentencia(generador, n->hi);
            generarSentencia(generador, n->hd);
            generarSentencia(generador, n->extra);
            break;
    }
}

static char* generarExpr(CodigoIntermedio* generador, Nodo* n) {
    // Implementacion pendiente
}


static char* nuevaTemp(CodigoIntermedio* generador) {
    char buffer[20];
    sprintf(buffer, "t%d", generador->tempCount++);
    return strdup(buffer);
}

static char* nuevoLabel(CodigoIntermedio* generador) {
    char buffer[20];
    sprintf(buffer, "L%d", generador->labelCount++);
    return strdup(buffer);
}

static const char* instrToStr(TipoInstr tipo) {
    switch (tipo) {
        case INSTR_ADD: return "ADD";
        case INSTR_SUB: return "SUB";
        case INSTR_MUL: return "MUL";
        case INSTR_DIV: return "DIV";
        case INSTR_MOD: return "MOD";
        case INSTR_LT:  return "LT";
        case INSTR_GT:  return "GT";
        case INSTR_EQ:  return "EQ";
        case INSTR_AND: return "AND";
        case INSTR_OR:  return "OR";
        case INSTR_NOT: return "NOT";
        case INSTR_ASSIGN: return "ASSIGN";
        case INSTR_IF_TRUE:  return "IF_TRUE";
        case INSTR_IF_FALSE: return "IF_FALSE";
        case INSTR_GOTO:     return "GOTO";
        case INSTR_LABEL:    return "LABEL";
        case INSTR_FUNC_BEGIN: return "FUNC_BEGIN";
        case INSTR_FUNC_END:   return "FUNC_END";
        case INSTR_PARAM:    return "PARAM";
        case INSTR_CALL:     return "CALL";
        case INSTR_RETVAL:   return "RETVAL";
        case INSTR_RETURN:   return "RETURN";
        case INSTR_VAR_GLOBAL: return "VAR_GLOBAL";
        case INSTR_VAR_LOCAL:  return "VAR_LOCAL";
        default: return "NOP";
    }
}

static void crearInstr(CodigoIntermedio* generador, TipoInstr tipo, const char* arg1, const char* arg2, const char* res, const char* label) {
    Instr* instr = malloc(sizeof(Instr));
    instr->tipo = tipo;

    if (arg1 != NULL) {
        instr->arg1 = strdup(arg1);
    } else {
        instr->arg1 = NULL;
    }

    if (arg2 != NULL) {
        instr->arg2 = strdup(arg2);
    } else {
        instr->arg2 = NULL;
    }

    if (res != NULL){
        instr->res = strdup(res);
    } else {
        instr->res = NULL;
    }

    if (label != NULL) {
        instr->label = strdup(label);
    } else {
        instr->label = NULL;
    }

    instr->next = NULL;

    if (generador->head == NULL) {
        generador->head = instr;
    } else {
        generador->tail->next = instr;
    }

    generador->tail = instr;
}