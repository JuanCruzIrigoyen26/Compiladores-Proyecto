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

void imprimir_ast(Nodo* nodo, int nivel) {
    if (!nodo) return;

    for (int i = 0; i < nivel; i++) printf("  ");

    switch (nodo->tipo) {
        case AST_PROG:
            printf("PROG\n");
            break;
        case AST_INT:
            printf("INT: %ld\n", nodo->v->i);
            break;
        case AST_BOOL:
            printf("BOOL: %s\n", nodo->v->b ? "true" : "false");
            break;
        case AST_ID:
            printf("ID: %s\n", nodo->v->s ? nodo->v->s : "(null)");
            break;
        case AST_OP:
            printf("OP: %s\n", op_str(nodo->v->op));
            break;
        case AST_DECL_VAR:
            printf("DECL_VAR: %s\n", nodo->v->s ? nodo->v->s : "(null)");
            break;
        case AST_DECL_FUNC:
            printf("DECL_FUNC: %s\n", nodo->v->s ? nodo->v->s : "(null)");
            break;
        case AST_PARAM:
            printf("PARAM: %s\n", nodo->v->s ? nodo->v->s : "(null)");
            break;
        case AST_PARAMS:
            printf("PARAMS\n");
            break;
        case AST_BLOQUE:
            printf("BLOQUE\n");
            break;
        case AST_DECLS:
            printf("DECLS\n");
            break;
        case AST_STMTS:
            printf("STMTS\n");
            break;
        case AST_SEQ_EXPR:
            printf("SEQ_EXPR\n");
            break;
        case AST_ASIGNACION:
            printf("ASIGNACION\n");
            break;
        case AST_IF:
            printf("IF\n");
            break;
        case AST_WHILE:
            printf("WHILE\n");
            break;
        case AST_RETURN:
            printf("RETURN\n");
            break;
        case AST_LLAMADA:
            if (nodo->hi && nodo->hi->tipo == AST_ID)
                printf("LLAMADA_FUNC: %s\n", nodo->hi->v->s);
            else
                printf("LLAMADA_FUNC: (null)\n");
            break;
        default:
            printf("Nodo desconocido\n");
        }

    imprimir_ast(nodo->hi, nivel + 1);
    imprimir_ast(nodo->hd, nivel + 1);
    imprimir_ast(nodo->extra, nivel + 1); // para ternarios (if-else)
}
