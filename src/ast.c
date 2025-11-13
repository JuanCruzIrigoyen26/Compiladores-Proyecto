#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// Función para crear un nodo hoja
Nodo* nodo_hoja(AstTipo t, AstValor v) {
    Nodo *n = (Nodo*)malloc(sizeof(Nodo));
    if (!n) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    n->tipo = t;
    n->hi = n->hd = n->extra = NULL;

    n->v = malloc(sizeof(AstValor));
    if (!n->v) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    *n->v = v;

    if ((t == AST_ID) && v.s) {
        n->v->s = strdup(v.s);
    }
    n->v->esExtern = v.esExtern;
    return n;
}

// Función para crear un nodo binario
Nodo* nodo_binario(AstTipo t, AstValor v, Nodo* hi, Nodo* hd) {
    Nodo *n = (Nodo*)malloc(sizeof(Nodo));
    if (!n) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    n->tipo = t;
    n->hi = hi;
    n->hd = hd;
    n->extra = NULL;

    n->v = malloc(sizeof(AstValor));
    if (!n->v) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    *n->v = v;
    n->v->esExtern = v.esExtern;
    return n;
}

// Función para crear un nodo ternario (por ejemplo if-else)
Nodo* nodo_ternario(AstTipo t, AstValor v, Nodo* hi, Nodo* hd, Nodo* extra) {
    Nodo *n = (Nodo*)malloc(sizeof(Nodo));
    if (!n) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    n->tipo = t;
    n->hi = hi;
    n->hd = hd;
    n->extra = extra;

    n->v = malloc(sizeof(AstValor));
    if (!n->v) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    *n->v = v;
    n->v->esExtern = v.esExtern;
    return n;
}

// Función auxiliar para convertir operadores a cadena
const char* op_str(TipoOp op) {
    switch(op){
        case OP_SUMA: return "+";
        case OP_RESTA: return "-";
        case OP_MULT: return "*";
        case OP_DIV: return "/";
        case OP_MOD: return "%";
        case OP_ASIG: return "=";
        case OP_IGUAL: return "==";
        case OP_MAYOR: return ">";
        case OP_MENOR: return "<";
        case OP_AND: return "&&";
        case OP_OR: return "||";
        case OP_NOT: return "!";
        default: return "?";
    }
}

void imprimir_ast(FILE *out, Nodo* nodo, int nivel) {
    if (!nodo) return;

    for (int i = 0; i < nivel; i++) fprintf(out, "  ");

    switch (nodo->tipo) {
        case AST_PROG:
            fprintf(out, "PROG\n");
            break;
        case AST_INT:
            fprintf(out, "INT: %ld\n", nodo->v->i);
            break;
        case AST_BOOL:
            fprintf(out, "BOOL: %s\n", nodo->v->b ? "true" : "false");
            break;
        case AST_ID:
            fprintf(out, "ID: %s\n", nodo->v->s ? nodo->v->s : "(null)");
            break;
        case AST_OP:
            fprintf(out, "OP: %s\n", op_str(nodo->v->op));
            break;
        case AST_DECL_VAR:
            fprintf(out, "DECL_VAR: %s\n", nodo->v->s ? nodo->v->s : "(null)");
            break;
        case AST_DECL_FUNC:
            fprintf(out, "DECL_FUNC: %s\n", nodo->v->s ? nodo->v->s : "(null)");
            break;
        case AST_PARAM:
            fprintf(out, "PARAM: %s\n", nodo->v->s ? nodo->v->s : "(null)");
            break;
        case AST_PARAMS:
            fprintf(out, "PARAMS\n");
            break;
        case AST_BLOQUE:
            fprintf(out, "BLOQUE\n");
            break;
        case AST_DECLS:
            fprintf(out, "DECLS\n");
            break;
        case AST_STMTS:
            fprintf(out, "STMTS\n");
            break;
        case AST_SEQ_EXPR:
            fprintf(out, "SEQ_EXPR\n");
            break;
        case AST_ASIGNACION:
            fprintf(out, "ASIGNACION\n");
            break;
        case AST_IF:
            fprintf(out, "IF\n");
            break;
        case AST_WHILE:
            fprintf(out, "WHILE\n");
            break;
        case AST_RETURN:
            fprintf(out, "RETURN\n");
            break;
        case AST_LLAMADA:
            if (nodo->hi && nodo->hi->tipo == AST_ID)
                fprintf(out, "LLAMADA_FUNC: %s\n", nodo->hi->v->s);
            else
                fprintf(out, "LLAMADA_FUNC: (null)\n");
            break;
        default:
            fprintf(out, "Nodo desconocido\n");
        }

    imprimir_ast(out, nodo->hi, nivel + 1);
    imprimir_ast(out, nodo->hd, nivel + 1);
    imprimir_ast(out, nodo->extra, nivel + 1); // para ternarios (if-else)
}


