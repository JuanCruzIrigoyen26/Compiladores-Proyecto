#include <stdio.h>
#include <stdlib.h>
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
        printf("\nGenerando archivo salida.txt...\n");
        FILE* out = fopen("salida.txt", "w");
        if (!out) {
            perror("Error creando salida.txt");
            exit(1);
        }
        generarCodigoObjeto(generador, out);
        fclose(out);
        printf("Código objeto guardado en salida.txt\n");

    }

    if (yyin != stdin){
        fclose(yyin);
    }

    return 0;
}
