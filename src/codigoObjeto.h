#ifndef CODIGO_OBJETO_H
#define CODIGO_OBJETO_H

#include "codigoIntermedio.h" 
#include <stdio.h>

// Función principal para generar código objeto a partir del código intermedio
void generarCodigoObjeto(CodigoIntermedio* generador, FILE* out);

#endif 
