#include <stdio.h>
#include "analizador_semantico.h"
#include "ts.h"

void errorSemantico(int linea, const char* msg) {
    fprintf(stderr, "Error semántico en línea %d: %s\n", linea, msg);
}

/* Función principal: recorre el AST y chequea las reglas */
void chequearSemantica(Nodo* nodo) {
    if (!nodo) return;

    chequearSemantica(nodo->hi);
    chequearSemantica(nodo->hd);
    chequearSemantica(nodo->extra);
    
    switch (nodo->tipo) {
        case AST_ID: 
            Simbolo* sim = buscarSimbolo(nodo->v->s);
            if (!sim) {
                errorSemantico(nodo->v->linea, "Identificador usado antes de ser declarado");
            } else {
                nodo->v->tipoDef = sim->v->tipoDef;  
            }
            break;
            
        case AST_IF:    
        case AST_WHILE:
            if (nodo->hi && nodo->hi->v->tipoDef != BOOL) {
                errorSemantico(nodo->hi->v->linea, "Condición de if/while debe ser de tipo bool");
            }
            break;
        
        case AST_OP:
        switch (nodo->v->op) {
            case OP_SUMA: case OP_RESTA: case OP_MULT: case OP_DIV: case OP_MOD:
                if (nodo->hi && nodo->hd) {
                    if (nodo->hi->v->tipoDef != INT || nodo->hd->v->tipoDef != INT) {
                        errorSemantico(nodo->v->linea, "Operandos deben ser enteros");
                    }
                }
                nodo->v->tipoDef = INT; 
                break;

            case OP_MAYOR: case OP_MENOR: case OP_IGUAL:
                if (nodo->hi && nodo->hd) {
                    if (nodo->hi->v->tipoDef != nodo->hd->v->tipoDef) {
                        errorSemantico(nodo->v->linea, "Operandos deben ser del mismo tipo");
                    }
                }
                nodo->v->tipoDef = BOOL; 
                break;

            case OP_AND: case OP_OR:
                if (nodo->hi && nodo->hi->v->tipoDef != BOOL) {
                    errorSemantico(nodo->v->linea, "Operandos de operador lógico deben ser bool");
                }
                if (nodo->hd && nodo->hd->v->tipoDef != BOOL) {
                    errorSemantico(nodo->v->linea, "Operandos de operador lógico deben ser bool");
                }
                nodo->v->tipoDef = BOOL; 
                break;

            case OP_NOT:
                if (nodo->hi && nodo->hi->v->tipoDef != BOOL) {
                    errorSemantico(nodo->v->linea, "Operando de NOT debe ser bool");
                }
                nodo->v->tipoDef = BOOL; 
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
}

