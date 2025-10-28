#include <stdio.h>
#include "ast.h"
#include "bison.tab.h"
#include "ts.h" 
#include "analizador_semantico.h"
#include "codigoIntermedio.h"
#include "codigoObjeto.h"  

extern Nodo* raiz;
extern FILE *yyin;  

int main(int argc, char *argv[]) {
    if (argc > 1)
        yyin = fopen(argv[1],"r");
    else
        yyin = stdin;

    inicializarTS(); // inicializa la tabla de símbolos

    if (yyparse() == 0) {
        printf("\nÁrbol de sintaxis abstracta (AST):\n");
        imprimir_ast(raiz, 0);
        imprimir_tabla();
        chequearSemantica(raiz);
        verificarMainFinal();
        CodigoIntermedio* generador = crearGenerador();
        generarCodigoIntermedio(generador, raiz);
        imprimirCodigoIntermedio(generador);
        // Generar código objeto y mostrar en terminal
        printf("\nCÓDIGO OBJETO:\n");
        generarCodigoObjeto(generador, stdout);
    }

    if (yyin != stdin){
        fclose(yyin);
    }

    return 0;
}
