#ifndef ANALIZADOR_SEMANTICO_H
#define ANALIZADOR_SEMANTICO_H

#include "ast.h"
#include "ts.h"

extern FILE* archivoSalidaSem;
extern int hayErrorSemantico;


void chequearSemantica(Nodo* nodo);
void verificarMainFinal();
#endif
