#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"
#include "bison.tab.h"
#include "ts.h" 
#include "analizador_semantico.h"
#include "codigoIntermedio.h"
#include "codigoObjeto.h"  

extern Nodo* raiz;
extern FILE *yyin;
extern int yylex(void);
extern char* yytext;
extern int yylineno;  
extern FILE* archivoSalidaSem;
extern int hayErrorSemantico;


typedef enum { 
    ETAPA_SCAN, 
    ETAPA_PARSE, 
    ETAPA_SEM, 
    ETAPA_CODINTER, 
    ETAPA_ASSEMBLY,
    ETAPA_OUT
} Etapa;

bool validarArchivoEntrada(const char* nombre) {
    // No puede empezar con '-'
    if (nombre[0] == '-') {
        fprintf(stderr, "Error: el nombre del archivo no puede comenzar con '-'\n");
        return false;
    }

    // Debe terminar con .ctds
    const char* ext = strrchr(nombre, '.');
    if (!ext || strcmp(ext, ".ctds") != 0) {
        fprintf(stderr, "Error: el archivo debe tener extensión .ctds\n");
        return false;
    }

    return true;
}

void generarNombreSalida(const char *entrada, const char *ext, char *salida) {
    strcpy(salida, entrada);
    char *punto = strrchr(salida, '.');
    if (punto && strcmp(punto, ".ctds") == 0)
        *punto = '\0';
    strcat(salida, ext);
}


int main(int argc, char *argv[]) {
    char* archivoEntrada = NULL;
    char* archivoSalida = NULL;
    Etapa etapa = ETAPA_OUT; // por defecto genera .out

    //determiar etapa
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-target") == 0 && i + 1 < argc) {
            i++;
            if (strcmp(argv[i], "scan") == 0)
                etapa = ETAPA_SCAN;
            else if (strcmp(argv[i], "parse") == 0)
                etapa = ETAPA_PARSE;
            else if (strcmp(argv[i], "sem") == 0)
                etapa = ETAPA_SEM;
            else if (strcmp(argv[i], "codinter") == 0)
                etapa = ETAPA_CODINTER;
            else if (strcmp(argv[i], "assembly") == 0)
                etapa = ETAPA_ASSEMBLY;
            else {
                fprintf(stderr, "Error: etapa '%s' desconocida.\n", argv[i]);
                return 1;
            }
        } 
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            archivoSalida = argv[++i];
        } 
        else {
            archivoEntrada = argv[i];
        }
    }

    //validar archivo de entrada
    if (!archivoEntrada) {
        fprintf(stderr, "Error: no se indicó archivo de entrada.\n");
        return 1;
    }

    if (!validarArchivoEntrada(archivoEntrada))
        return 1;

    yyin = fopen(archivoEntrada, "r");
    if (!yyin) {
        perror("Error al abrir el archivo de entrada");
        return 1;
    }

    //determinar nombre de salida
    char nombreSalidaAuto[256];
    if (!archivoSalida) {
        switch (etapa) {
            case ETAPA_SCAN: generarNombreSalida(archivoEntrada, ".lex", nombreSalidaAuto); break;
            case ETAPA_PARSE: generarNombreSalida(archivoEntrada, ".sint", nombreSalidaAuto); break;
            case ETAPA_SEM: generarNombreSalida(archivoEntrada, ".sem", nombreSalidaAuto); break;
            case ETAPA_CODINTER: generarNombreSalida(archivoEntrada, ".ci", nombreSalidaAuto); break;
            case ETAPA_ASSEMBLY: generarNombreSalida(archivoEntrada, ".ass", nombreSalidaAuto); break;
            case ETAPA_OUT: generarNombreSalida(archivoEntrada, ".out", nombreSalidaAuto); break;
        }
        archivoSalida = nombreSalidaAuto;
    }
    //ejecucion segun la etapa
    switch (etapa) {
        case ETAPA_SCAN: {
             FILE* out = fopen(archivoSalida, "w");
            if (!out) { perror("Error al crear archivo de salida"); break; }
            printf("Etapa: SCAN → generando tokens en %s\n", archivoSalida);
            int token;
            while ((token = yylex()) != 0) {
                fprintf(out, "TOKEN: %-3d | Texto: %-15s | Línea: %d\n",
                        token, yytext, yylineno);
            }
            fclose(out);
            printf("Archivo generado: %s \n", archivoSalida);
            break;
        }

        case ETAPA_PARSE: {
            FILE* out = fopen(archivoSalida, "w");
            if (!out) { perror("Error al crear archivo de salida"); break; }
            printf("Etapa: PARSE → generando árbol sintáctico en %s\n", archivoSalida);
            // Llamada al parser
            inicializarTS();
            if (yyparse() == 0) {   // si parseó correctamente
                imprimir_ast(out, raiz, 0);  // imprimir el AST en el archivo de salida
            } else {
                fprintf(stderr, "Error: fallo en el parseo.\n");
            }
            fclose(out);
            printf("Archivo generado: %s \n", archivoSalida);
            break;
        }

        case ETAPA_SEM: {
            FILE* out = fopen(archivoSalida, "w");
            if (!out) { perror("Error al crear archivo de salida"); break; }

            printf("Etapa: SEM → generando análisis en %s\n", archivoSalida);

            inicializarTS();     // tabla vacía
            archivoSalidaSem = out; // para que errorSemantico escriba en el archivo

            if (yyparse() == 0) {
                fprintf(out, "Árbol Sintáctico:\n");
                imprimir_ast(out, raiz, 0);

                fprintf(out, "\nComienzo de análisis semántico.\n");
                chequearSemantica(raiz);
                verificarMainFinal();

                fprintf(out, "\nTabla de Símbolos (TS):\n");
                imprimir_tabla(out);

                fprintf(out, "\nAnálisis semántico finalizado.\n");
            } else {
                fprintf(out, "Error: Parsing falló. No se realizó análisis semántico.\n");
            }

            fclose(out);
            printf("Archivo generado: %s \n", archivoSalida);
            break;
        }
        case ETAPA_CODINTER: {
            printf("Etapa: CODINTER → generando código intermedio en %s\n", archivoSalida);

            inicializarTS();
            archivoSalidaSem = NULL;
            hayErrorSemantico = 0;

            if (yyparse() == 0) {
                chequearSemantica(raiz);
                verificarMainFinal();

                if (hayErrorSemantico) {
                    fprintf(stderr, "Se detectaron errores semánticos. No se generará el código intermedio.\n");
                }else{
                    FILE* out = fopen(archivoSalida, "w");
                    if (!out) {
                        perror("Error al crear archivo de salida");
                        break;
                    }
                    CodigoIntermedio* gen = crearGenerador();
                    generarCodigoIntermedio(gen, raiz);
                    imprimirCodigoIntermedio(out, gen); 
                    fclose(out);
                    printf("Código intermedio generado correctamente: %s\n", archivoSalida);
                }

            } else {
                fprintf(stderr, "Error: Parsing falló. No se generó código intermedio.\n");
            }
            break;
        }

        case ETAPA_ASSEMBLY: {
            printf("Etapa: ASSEMBLY → generando código ensamblador en %s\n", archivoSalida);

            inicializarTS();
            archivoSalidaSem = NULL;
            hayErrorSemantico = 0;

            if (yyparse() == 0) {
                chequearSemantica(raiz);
                verificarMainFinal();

                if (hayErrorSemantico) {
                    fprintf(stderr, "Se detectaron errores semánticos. No se generará el código ensamblador.\n");
                } else {
                    FILE* out = fopen(archivoSalida, "w");
                    if (!out) {
                        perror("Error al crear archivo de salida");
                        break;
                    }

                    CodigoIntermedio* gen = crearGenerador();
                    generarCodigoIntermedio(gen, raiz);

                    // Generar código objeto (assembly)
                    generarCodigoObjeto(gen, out);
                    fclose(out);
                    printf("Código ensamblador generado correctamente: %s\n", archivoSalida);
                }
            } else {
                fprintf(stderr, "Error: Parsing falló. No se generó código ensamblador.\n");
            }
            break;
        }

        case ETAPA_OUT: {
            printf("Etapa: OUT → generando ejecutable final\n");

            inicializarTS();
            archivoSalidaSem = NULL;
            hayErrorSemantico = 0;

            if (yyparse() == 0) {
                chequearSemantica(raiz);
                verificarMainFinal();

                if (hayErrorSemantico) {
                    fprintf(stderr, "Se detectaron errores semánticos. No se generará el ejecutable.\n");
                } else {
                    char archivoAsm[256];
                    generarNombreSalida(archivoEntrada, ".s", archivoAsm);

                    FILE* outAsm = fopen(archivoAsm, "w");
                    if (!outAsm) {
                        perror("Error al crear archivo assembler");
                        break;
                    }

                    CodigoIntermedio* gen = crearGenerador();
                    generarCodigoIntermedio(gen, raiz);
                    generarCodigoObjeto(gen, outAsm);
                    fclose(outAsm);

                    char ejecutable[256];
                    strcpy(ejecutable, archivoEntrada);
                    char* punto = strrchr(ejecutable, '.');
                    if (punto) *punto = '\0'; 

                    char comando[512];
                    sprintf(comando, "gcc -no-pie -o %s %s", ejecutable, archivoAsm);

                    int r = system(comando);
                    if (r != 0) {
                        fprintf(stderr, "Error: GCC falló compilando el ejecutable.\n");
                    } else {
                        printf("Ejecutable generado correctamente: %s\n", ejecutable);
                    }
                }
            } else {
                fprintf(stderr, "Error: Parsing falló. No se generó ejecutable.\n");
            }
            break;
        }

    }
    fclose(yyin);
    return 0;
}
