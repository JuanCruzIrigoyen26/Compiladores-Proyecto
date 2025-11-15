#ifndef TS_H
#define TS_H

#include "ast.h"

typedef struct Simbolo {
    AstValor *v;
    struct Simbolo *sig;
} Simbolo;

typedef struct Nivel {
    Simbolo *tabla;        // Lista de símbolos de este nivel
    struct Nivel *sig;     // Siguiente nivel (anterior en la pila)
} Nivel;

// Operaciones sobre la tabla de símbolos
void inicializarTS();
void abrirNivel();
void cerrarNivel();
Simbolo* insertarSimbolo(AstValor *valor);
Simbolo* buscarSimbolo(char *nombre);
void imprimir_tabla(FILE *out);
const char* tipoAString(tipoDef t);
void prepararParametros();
void insertarParametros(struct Nodo* params);
int existeEnNivelActual(const char* nombre);

#endif
