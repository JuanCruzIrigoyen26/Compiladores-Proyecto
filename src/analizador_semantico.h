#ifndef ANALIZADOR_SEMANTICO_H
#define ANALIZADOR_SEMANTICO_H

#include "ast.h"
#include "ts.h"

extern FILE* archivoSalidaSem;

void chequearSemantica(Nodo* nodo);
void verificarMainFinal();
#endif
