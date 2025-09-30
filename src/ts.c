#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ts.h"

static Nivel *pila = NULL;
static Nivel *pilaHistorica = NULL;

// Inicializa la pila vacía
void inicializarTS() {
    pila = NULL;
    abrirNivel();
}

// Abre un nuevo nivel
void abrirNivel() {
    Nivel *nuevoNivel = (Nivel*)malloc(sizeof(Nivel));
    if (!nuevoNivel) {
        fprintf(stderr, "Error: sin memoria para nivel\n");
        exit(1);
    }
    nuevoNivel->tabla = NULL;
    nuevoNivel->sig = pila;
    pila = nuevoNivel;
}

// Cierra el nivel actual
void cerrarNivel() {
    if (!pila) return;
    Nivel *nivelCerrado = pila;
    pila = pila->sig;

    nivelCerrado->sig = pilaHistorica;
    pilaHistorica = nivelCerrado;
}

// Inserta un símbolo en el nivel actual
Simbolo* insertarSimbolo(AstValor *valor) {
    if (!pila) {
        fprintf(stderr, "Error: no hay nivel abierto\n");
        return NULL;
    }

    AstValor *nuevoVal = malloc(sizeof(AstValor));
    *nuevoVal = *valor;
    if (valor->s) nuevoVal->s = strdup(valor->s);

    Simbolo *nuevoSimbolo = malloc(sizeof(Simbolo));
    nuevoSimbolo->v = nuevoVal;
    nuevoSimbolo->sig = pila->tabla;
    pila->tabla = nuevoSimbolo;

    return nuevoSimbolo;
}


// Busca un símbolo desde el nivel actual hacia arriba
Simbolo* buscarSimbolo(char *nombre) {
    Nivel *n = pila;
    while (n) {
        Simbolo *simbolo = n->tabla;
        while (simbolo) {
            if (strcmp(simbolo->v->s, nombre) == 0){
                return simbolo;
            }
            simbolo = simbolo->sig;
        }
        n = n->sig;
    }
    Nivel *nc = pilaHistorica;
    while (nc) {
        Simbolo *simbolo = nc->tabla;
        while (simbolo) {
            if (strcmp(simbolo->v->s, nombre) == 0)
                return simbolo;
            simbolo = simbolo->sig;
        }
        nc = nc->sig;
    }
    return NULL;
}

// Imprime todas las tablas
void imprimir_tabla() {
    printf("Tabla de Simbolos (TS):\n");

    int nivelActivo = 0;
    Nivel *aux = pila;
    while (aux) {
        if (aux->tabla)
            printf("Nivel activo %d:\n", nivelActivo);

        Simbolo *simbolo = aux->tabla;
        while (simbolo) {
            if (simbolo->v->esFuncion) {
                if (simbolo->v->tipoDef == INT)
                    printf("  %s (función, int)\n", simbolo->v->s);
                else if (simbolo->v->tipoDef == BOOL)
                    printf("  %s (función, bool)\n", simbolo->v->s);
                else if (simbolo->v->tipoDef == VOID)
                    printf("  %s (función, void)\n", simbolo->v->s);
                else
                    printf("  %s (función, tipo desconocido)\n", simbolo->v->s);
            } else {
                if (simbolo->v->tipoDef == INT) {
                    printf("  %s : int", simbolo->v->s);
                    printf(" = %ld\n", simbolo->v->i);
                } else if (simbolo->v->tipoDef == BOOL) {
                    printf("  %s : bool", simbolo->v->s);
                    printf(" = %s\n", simbolo->v->b ? "true" : "false");
                } else {
                    printf("  %s (tipo desconocido)\n", simbolo->v->s);
                }
            }
            simbolo = simbolo->sig;
        }

        aux = aux->sig;
        nivelActivo++;
    }

    int nivelCerrado = 0;
    aux = pilaHistorica;
    while (aux) {
        if (aux->tabla)
            printf("Nivel cerrado %d:\n", nivelCerrado);

        Simbolo *simbolo = aux->tabla;
        while (simbolo) {
            if (simbolo->v->esFuncion) {
                if (simbolo->v->tipoDef == INT)
                    printf("  %s (función, int)\n", simbolo->v->s);
                else if (simbolo->v->tipoDef == BOOL)
                    printf("  %s (función, bool)\n", simbolo->v->s);
                else if (simbolo->v->tipoDef == VOID)
                    printf("  %s (función, void)\n", simbolo->v->s);
                else
                    printf("  %s (función, tipo desconocido)\n", simbolo->v->s);
            } else {
                if (simbolo->v->tipoDef == INT){
                    printf("  %s : int", simbolo->v->s);
                    printf(" = %ld\n", simbolo->v->i);
                }else if (simbolo->v->tipoDef == BOOL) {
                    printf("  %s : bool", simbolo->v->s);
                    printf(" = %s\n", simbolo->v->b ? "true" : "false");
                }else{
                    printf("  %s (tipo desconocido)\n", simbolo->v->s);
                }
            }
            simbolo = simbolo->sig;
        }

        aux = aux->sig;
        nivelCerrado++;
    }
}
// Funcion auxiliar para insertar los parametros en la tabla
void insertarParametros(Nodo* params) {
    if (!params) return;

    if (params->tipo == AST_PARAM) {
        insertarSimbolo(params->v);
    }
    else if (params->tipo == AST_PARAMS) {
        insertarParametros(params->hi);
        insertarParametros(params->hd);
    }
}