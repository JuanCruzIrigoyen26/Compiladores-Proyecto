#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ts.h"

static Nivel *pila = NULL;

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

    // Liberar símbolos del nivel
    Simbolo *s = nivelCerrado->tabla;
    while (s) {
        Simbolo *aux = s;
        s = s->sig;
        free(aux->v->s); // liberar nombre
        free(aux->v);    // liberar valor asociado
        free(aux);
    }
    free(nivelCerrado);
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
    return NULL;
}

// Imprime todas las tablas
void imprimir_tabla() {
    int totalNiveles = 0;
    Nivel *aux = pila;
    while (aux) {
        totalNiveles++;
        aux = aux->sig;
    }

    int nivelActual = totalNiveles - 1;
    aux = pila;
    while (aux) {
        printf("Nivel %d:\n", nivelActual);

        Simbolo *simbolo = aux->tabla;
        while (simbolo) {
            if (simbolo->v->esFuncion) {
                if (simbolo->v->tipoDef == INT) {
                    printf("  %s (función, int)\n", simbolo->v->s);
                } else if (simbolo->v->tipoDef == BOOL) {
                    printf("  %s (función, bool)\n", simbolo->v->s);
                } else if (simbolo->v->tipoDef == VOID) {
                    printf("  %s (función, void)\n", simbolo->v->s);
                } else {
                    printf("  %s (tipo desconocido)\n", simbolo->v->s);
                }
                
            }else if (!(simbolo->v->esFuncion)) {
                if (simbolo->v->tipoDef == INT) {
                    printf("  %s : int\n", simbolo->v->s);
                } else if (simbolo->v->tipoDef == BOOL) {
                    printf("  %s : bool\n", simbolo->v->s);
                } else {
                    printf("  %s (tipo desconocido)\n", simbolo->v->s);
                }
            }
            simbolo = simbolo->sig;
        }

        aux = aux->sig;
        nivelActual--;
    }
}


