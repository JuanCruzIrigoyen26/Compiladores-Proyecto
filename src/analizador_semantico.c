#include <stdio.h>
#include "analizador_semantico.h"
#include "ts.h"

void errorSemantico(int linea, const char* msg) {
    fprintf(stderr, "Error semántico en línea %d: %s\n", linea, msg);
}

/* Función principal: recorre el AST y chequea las reglas */
void chequearSemantica(Nodo* nodo) {
    if (!nodo) return;

    switch (nodo->tipo) {
        case AST_ID:
        if (!buscarSimbolo(nodo->v->s)) {
            errorSemantico(nodo->v->linea, "Identificador usado antes de ser declarado");
        }
            break;
            
        case AST_WHILE:
            if (nodo->hi && nodo->hi->v->tipoDef != BOOL) {
                errorSemantico(nodo->hi->v->linea, "Condición de if/while debe ser de tipo bool");
            }
            break;
        
        case AST_OP:
            switch (nodo->v->op) {
                case OP_SUMA: case OP_RESTA: case OP_MULT: case OP_DIV: case OP_MOD:
                case OP_MAYOR: case OP_MENOR:
                    if (nodo->hi && nodo->hd) {
                        if (nodo->hi->v->tipoDef != INT || nodo->hd->v->tipoDef != INT) {
                            errorSemantico(nodo->v->linea, "Operandos deben ser enteros");
                        }
                    }
                    break;
       
                case OP_IGUAL:
                    if (nodo->hi && nodo->hd) {
                        if (nodo->hi->v->tipoDef != nodo->hd->v->tipoDef) {
                            errorSemantico(nodo->v->linea, "Operandos de == deben ser del mismo tipo");
                        }
                    }
                    break;
                
                case OP_AND: case OP_OR: case OP_NOT:
                    if (nodo->hi && nodo->hi->v->tipoDef != BOOL) {
                        errorSemantico(nodo->v->linea, "Operandos de operador lógico deben ser bool");
                    }
                    if (nodo->hd && nodo->hd->v->tipoDef != BOOL) {
                        errorSemantico(nodo->v->linea, "Operandos de operador lógico deben ser bool");
                    }
                    break;
                default:
                    break;
            }
            break;    

        case AST_ASIGNACION:
            if (nodo->hi && nodo->hd) {
                int tipoLoc = nodo->hi->v->tipoDef;
                int tipoExpr = nodo->hd->v->tipoDef;
                if (tipoLoc != tipoExpr) {
                    errorSemantico(nodo->v->linea, "Tipo de la asignación no coincide con la variable");
                }
            }
            break;

        default:
            break;
    }

    // Recursión sobre hijos
    chequearSemantica(nodo->hi);
    chequearSemantica(nodo->hd);
    chequearSemantica(nodo->extra);
}
