#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ts.h"

static Nivel *pila = NULL;
static Nivel *pilaHistorica = NULL;
static int contadorParams = 0;

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
    for (Nivel *n = pila; n; n = n->sig) {
        for (Simbolo *s = n->tabla; s; s = s->sig) {
            if (s->v->s && strcmp(s->v->s, nombre) == 0) {
                return s;
            }
        }
    }
    return NULL; // no mirar pilaHistorica
}


// Imprime todas las tablas
void imprimir_tabla(FILE *out) {
    int idx = 0;
    // 1) Activos (de pila, desde el actual hacia arriba)
    for (Nivel *aux = pila; aux; aux = aux->sig) {
        if (!aux->tabla) continue;                 // omitir vacíos
        fprintf(out, "Nivel %d:\n", idx++);
        for (Simbolo *s = aux->tabla; s; s = s->sig) {
            if (s->v->esFuncion) {
                const char *t = (s->v->tipoDef == INT ? "int" :
                                 s->v->tipoDef == BOOL ? "bool" :
                                 s->v->tipoDef == VOID ? "void" : "tipo desconocido");
                fprintf(out, "  %s (función, %s)\n", s->v->s, t);
            } else {
                if (s->v->tipoDef == INT)
                    fprintf(out, "  %s : int\n", s->v->s);
                else if (s->v->tipoDef == BOOL)
                    fprintf(out, "  %s : bool\n", s->v->s);
                else
                    fprintf(out, "  %s (tipo desconocido)\n", s->v->s);
            }
        }
    }

    // 2) Cerrados (históricos). Imprimimos en orden de cierre (como los guardaste)
    for (Nivel *aux = pilaHistorica; aux; aux = aux->sig) {
        if (!aux->tabla) continue;                 // omitir vacíos
        fprintf(out, "Nivel %d:\n", idx++);
        for (Simbolo *s = aux->tabla; s; s = s->sig) {
            if (s->v->esFuncion) {
                const char *t = (s->v->tipoDef == INT ? "int" :
                                 s->v->tipoDef == BOOL ? "bool" :
                                 s->v->tipoDef == VOID ? "void" : "tipo desconocido");
                fprintf(out, "  %s (función, %s)\n", s->v->s, t);
            } else {
                if (s->v->tipoDef == INT)
                    fprintf(out, "  %s : int\n", s->v->s);
                else if (s->v->tipoDef == BOOL)
                    fprintf(out, "  %s : bool\n", s->v->s);
                else
                    fprintf(out, "  %s (tipo desconocido)\n", s->v->s);
            }
        }
    }
}



void prepararParametros() {
    contadorParams = 0;
}


// Funcion auxiliar para insertar los parametros en la tabla
void insertarParametros(Nodo* params) {
    if (!params) return;

    if (params->tipo == AST_PARAM) {
        Simbolo* s = insertarSimbolo(params->v);
        s->v->offset = contadorParams;
        s->v->esParametro = 1;
        contadorParams++;
    }
    else if (params->tipo == AST_PARAMS) {
        insertarParametros(params->hi);
        insertarParametros(params->hd);
    }
}

// Chequea si el simbolo existe en el bloque actual
int existeEnNivelActual(const char* nombre) {
    if (!pila || !nombre) return 0;
    Simbolo *sim = pila->tabla;
    while (sim) {
        if (sim->v && sim->v->s && strcmp(sim->v->s, nombre) == 0) {
            return 1;
        }
        sim = sim->sig;
    }
    return 0;
}
