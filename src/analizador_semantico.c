#include <stdio.h>
#include <string.h>
#include "analizador_semantico.h"
#include "ts.h"

extern Nodo* raiz;  // Para acceder al AST completo y encontrar declaraciones de funciones por nombre
static int existeMainFlag = 0;  // Flag para chequear si la funcion main se encuentra en el programa
static int tipoFuncionActual = VOID;    // Tipo de función actual
static void chequearNodo(Nodo* nodo);
static void chequearMainDecl(Nodo* declFunc);
static void chequearLlamada(Nodo* llamada);
static Nodo* buscarDeclFuncionEnAST(Nodo* n, const char* nombre);
static int contarParametrosAST(Nodo* params);
static void llenarTiposParametros(Nodo* params, tipoDef* arrayTipos, int* indice, int max);
static int contarArgumentosAST(Nodo* argumentos);
static void llenarTiposArgumentos(Nodo* argumentos, tipoDef* arrayArguments, int* indice, int max);

FILE* archivoSalidaSem = NULL;

static int lineaDe(Nodo* n) {
    if (!n) return 0;
    if (n->v && n->v->linea > 0) return n->v->linea;
    int l = 0;
    l = lineaDe(n->hi); if (l) return l;
    l = lineaDe(n->hd); if (l) return l;
    l = lineaDe(n->extra); return l;
}

// Funcion que muestra el error semantico encontrado
void errorSemantico(int linea, const char* msg) {
    fprintf(archivoSalidaSem, "Error semántico en línea %d: %s\n", linea, msg);
}

// Funcion principal 
void chequearSemantica(Nodo* nodo) {
    chequearNodo(nodo);
}

// Funcion que chequea si el programa contiene la funcion main (sin parámetros) 
void verificarMainFinal() {
    if (!existeMainFlag) {
        errorSemantico(0, "El programa debe contener una función main sin parámetros");
    }
}

// Chequeos de operaciones
static void chequearOp(Nodo* nodo) {
    if (!nodo) return;
    if (nodo->hi) chequearNodo(nodo->hi);
    if (nodo->hd) chequearNodo(nodo->hd);

    switch (nodo->v->op) {
        case OP_SUMA: case OP_RESTA: case OP_MULT: case OP_DIV: case OP_MOD: {
            if (!nodo->hi || !nodo->hd ||
                nodo->hi->v->tipoDef != INT || nodo->hd->v->tipoDef != INT) {
                errorSemantico(lineaDe(nodo), "Operandos deben ser enteros");
            }
            nodo->v->tipoDef = INT;
            break;
        }
        case OP_MAYOR: case OP_MENOR: {
            if (!nodo->hi || !nodo->hd ||
                nodo->hi->v->tipoDef != INT || nodo->hd->v->tipoDef != INT) {
                errorSemantico(lineaDe(nodo), "Operandos relacionales deben ser enteros");
            }
            nodo->v->tipoDef = BOOL;
            break;
        }
        case OP_IGUAL: {
            if (!nodo->hi || !nodo->hd) {
                errorSemantico(lineaDe(nodo), "Comparación de igualdad incompleta");
            } else if (nodo->hi->v->tipoDef != nodo->hd->v->tipoDef) {
                errorSemantico(lineaDe(nodo), "Operandos de == deben ser del mismo tipo");
            }
            nodo->v->tipoDef = BOOL;
            break;
        }
        case OP_AND: case OP_OR: {
            if (!nodo->hi || !nodo->hd ||
                nodo->hi->v->tipoDef != BOOL || nodo->hd->v->tipoDef != BOOL) {
                errorSemantico(lineaDe(nodo), "Operandos lógicos deben ser bool");
            }
            nodo->v->tipoDef = BOOL;
            break;
        }
        case OP_NOT: {
            if (!nodo->hi || nodo->hi->v->tipoDef != BOOL) {
                errorSemantico(lineaDe(nodo), "Operando de ! debe ser bool");
            }
            nodo->v->tipoDef = BOOL;
            break;
        }
        default: break;
    }
}

// Chequeo de bloque
static void chequearBloque(Nodo* bloque) {
    if (!bloque) return;

    // si es cuerpo de función → ya abrimos nivel antes
    if (bloque->esBloqueDeFuncion) {
        if (bloque->hi) chequearNodo(bloque->hi);
        if (bloque->hd) chequearNodo(bloque->hd);
        if (bloque->extra) chequearNodo(bloque->extra);
        return;
    }

    // Si es bloque normal → if / while
    abrirNivel();
    if (bloque->hi) chequearNodo(bloque->hi);
    if (bloque->hd) chequearNodo(bloque->hd);
    if (bloque->extra) chequearNodo(bloque->extra);
    cerrarNivel();
}

// Chequeo de lista de sequencias, declaraciones y sentencias
static void chequearSeq(Nodo* lista) {
    if (!lista) return;
    if (lista->hi) chequearNodo(lista->hi);
    if (lista->hd) chequearNodo(lista->hd);
}

// Chequeo principal de nodo
static void chequearNodo(Nodo* nodo) {
    if (!nodo) return;

    switch (nodo->tipo) {
        case AST_PROG: {
            if (nodo->hi) chequearNodo(nodo->hi);
            if (nodo->hd) chequearNodo(nodo->hd);
            if (nodo->extra) chequearNodo(nodo->extra);
            break;
        }
        case AST_DECLS:
        case AST_STMTS:
        case AST_SEQ_EXPR: {
            chequearSeq(nodo);
            break;
        }
        case AST_BLOQUE: {
            chequearBloque(nodo);
            break;
        }
        case AST_DECL_VAR: {
            
            if (nodo->hi) chequearNodo(nodo->hi);
            if (nodo->hd) chequearNodo(nodo->hd);

            
            if (existeEnNivelActual(nodo->v->s)) {
                errorSemantico(nodo->v->linea, "Identificador declarado dos veces en el mismo bloque");
            } else {
                insertarSimbolo(nodo->v); 
            }

            
            if (nodo->hd && nodo->hd->v->tipoDef != nodo->v->tipoDef) {
                errorSemantico(nodo->v->linea, "Tipo de inicialización no coincide con el de la variable");
            }
            break;
        }
        case AST_DECL_FUNC: {
            insertarSimbolo(nodo->v);
            chequearMainDecl(nodo);
            int tipoAnterior = tipoFuncionActual;
            tipoFuncionActual = nodo->v->tipoDef;

            abrirNivel();
            insertarParametros(nodo->hi);

            if (nodo->hd) chequearNodo(nodo->hd);

            cerrarNivel();
            tipoFuncionActual = tipoAnterior;
            break;
        }
        case AST_ID: {
            if (!nodo->v || !nodo->v->s) break;
            Simbolo* sim = buscarSimbolo(nodo->v->s);
            if (!sim) {
                errorSemantico(nodo->v->linea, "Identificador usado antes de ser declarado");
            } else {
                nodo->v->tipoDef = sim->v->tipoDef;
            }
            break;
        }
        case AST_INT: {
            nodo->v->tipoDef = INT;
            break;
        }
        case AST_BOOL: {
            nodo->v->tipoDef = BOOL;
            break;
        }
        case AST_OP: {
            chequearOp(nodo);
            break;
        }
        case AST_ASIGNACION: {
            if (nodo->hi) chequearNodo(nodo->hi);
            if (nodo->hd) chequearNodo(nodo->hd);
            if (nodo->hi && nodo->hd &&
                nodo->hi->v->tipoDef != nodo->hd->v->tipoDef) {
                errorSemantico(lineaDe(nodo->hi), "Tipo de la asignación no coincide con la variable");
            }
            break;
        }
        case AST_IF:{
            if (nodo->hi) chequearNodo(nodo->hi);
            if (!nodo->hi || nodo->hi->v->tipoDef != BOOL) {
                errorSemantico(lineaDe(nodo->hi), "Condición de if debe ser bool");
            }
            if (nodo->hd) chequearNodo(nodo->hd);
            if (nodo->extra) chequearNodo(nodo->extra);
            break;
        }
        case AST_WHILE: {
            if (nodo->hi) chequearNodo(nodo->hi);
            if (!nodo->hi || nodo->hi->v->tipoDef != BOOL) {
                errorSemantico(lineaDe(nodo->hi), "Condición de while debe ser bool");
            }
            if (nodo->hd) chequearNodo(nodo->hd);
            break;
        }
        case AST_LLAMADA: {
            if (nodo->hi) chequearNodo(nodo->hi);
            if (nodo->hd) chequearNodo(nodo->hd);
            chequearLlamada(nodo);
            break;
        }
        case AST_RETURN: {
            if (nodo->hi) chequearNodo(nodo->hi);
            if (tipoFuncionActual == VOID) {
                if (nodo->hi) {
                    errorSemantico(lineaDe(nodo), "Return en función void no debe devolver expresión");
                }
            } else {
                if (!nodo->hi) {
                    errorSemantico(lineaDe(nodo), "Return en función con valor debe devolver expresión");
                } else if (nodo->hi->v->tipoDef != tipoFuncionActual) {
                    errorSemantico(lineaDe(nodo), "Tipo de retorno no coincide con lo declarado");
                }
            }
            break;
        }
        default: {
            if (nodo->hi) chequearNodo(nodo->hi);
            if (nodo->hd) chequearNodo(nodo->hd);
            if (nodo->extra) chequearNodo(nodo->extra);
            break;
        }
    }
}

// Verifica propiedades de main en el momento de declarar 
static void chequearMainDecl(Nodo* declFunc) {
    if (!declFunc || !declFunc->v || !declFunc->v->s) return;
    if (strcmp(declFunc->v->s, "main") != 0) return;

    if (existeMainFlag) {
        errorSemantico(declFunc->v->linea, "El programa no puede tener más de un main");
    }
    existeMainFlag = 1;

    int cantParams = contarParametrosAST(declFunc->hi);
    if (cantParams != 0) {
        errorSemantico(declFunc->v->linea, "El método main no debe tener parámetros");
    }
}

// Chequeo de número y tipos de argumentos en invocación/llamada de función 
static void chequearLlamada(Nodo* llamada) {
    if (!llamada || llamada->tipo != AST_LLAMADA) return;
    if (!llamada->hi || llamada->hi->tipo != AST_ID || !llamada->hi->v || !llamada->hi->v->s) return;

    const char* nombre = llamada->hi->v->s;

    // Busco si existe un símbolo con ese nombre y que sea función 
    Simbolo* simbolo = buscarSimbolo((char*)nombre);
    if (!simbolo) {
        errorSemantico(lineaDe(llamada->hi), "Función invocada no declarada");
        return;
    }
    if (!simbolo->v->esFuncion) {
        errorSemantico(lineaDe(llamada->hi), "El identificador invocado no es una función");
        return;
    }

    // Seteo el tipo de retorno en la llamada 
    llamada->v->tipoDef = simbolo->v->tipoDef;

    // Comparo numero y tipos con la declaración en el AST 
    Nodo* decl = buscarDeclFuncionEnAST(raiz, nombre);
    if (!decl) return;

    int nParams = contarParametrosAST(decl->hi);
    int nArgumentos = contarArgumentosAST(llamada->hd);
    if (nParams != nArgumentos) {
        errorSemantico(lineaDe(llamada->hi), "Cantidad de argumentos no coincide con la definición");
        return;
    }

    tipoDef tiposParams[64];
    tipoDef tiposArgumentos[64];
    int indiceParametros = 0, indiceArgumentos = 0;
    llenarTiposParametros(decl->hi, tiposParams, &indiceParametros, 64);
    llenarTiposArgumentos(llamada->hd, tiposArgumentos, &indiceArgumentos, 64);
    for (int i = 0; i < nParams && i < indiceArgumentos; ++i) {
        if (tiposParams[i] != tiposArgumentos[i]) {
            errorSemantico(lineaDe(llamada->hi), "Tipo de argumento no coincide con el parámetro");
        }
    }
}

// Busca en el AST la declaración de función con un nombre dado 
static Nodo* buscarDeclFuncionEnAST(Nodo* n, const char* nombre) {
    if (!n) return NULL;
    if (n->tipo == AST_DECL_FUNC && n->v && n->v->s && strcmp(n->v->s, nombre) == 0) {
        return n;
    }
    Nodo* nodoRecursivo = NULL;
    if (!nodoRecursivo) nodoRecursivo = buscarDeclFuncionEnAST(n->hi, nombre);
    if (!nodoRecursivo) nodoRecursivo = buscarDeclFuncionEnAST(n->hd, nombre);
    if (!nodoRecursivo) nodoRecursivo = buscarDeclFuncionEnAST(n->extra, nombre);
    return nodoRecursivo;
}

// Cuenta la cantidad de parametros en la declaracion de funcion
static int contarParametrosAST(Nodo* params) {
    if (!params) return 0;
    if (params->tipo == AST_PARAM) return 1;
    if (params->tipo == AST_PARAMS) {
        return contarParametrosAST(params->hi) + contarParametrosAST(params->hd);
    }
    return 0;
}

// Guardo el tipo de cada parametro en orden
static void llenarTiposParametros(Nodo* params, tipoDef* arrayTipos, int* indice, int max) {
    if (!params || *indice >= max) return;
    if (params->tipo == AST_PARAM) {
        arrayTipos[(*indice)++] = params->v->tipoDef;
    } else if (params->tipo == AST_PARAMS) {
        llenarTiposParametros(params->hi, arrayTipos, indice, max);
        llenarTiposParametros(params->hd, arrayTipos, indice, max);
    }
}

// Cuenta la cantidad de argumentos en llamada a funcion
static int contarArgumentosAST(Nodo* argumentos) {
    if (!argumentos) return 0;
    if (argumentos->tipo == AST_SEQ_EXPR) {
        return contarArgumentosAST(argumentos->hi) + contarArgumentosAST(argumentos->hd);
    }
    return 1;
}

// Guardo el tipo de cada argumento en orden
static void llenarTiposArgumentos(Nodo* argumentos, tipoDef* arrayArguments, int* indice, int max) {
    if (!argumentos || *indice >= max) return;
    if (argumentos->tipo == AST_SEQ_EXPR) {
        llenarTiposArgumentos(argumentos->hi, arrayArguments, indice, max);
        llenarTiposArgumentos(argumentos->hd, arrayArguments, indice, max);
    } else {
        arrayArguments[(*indice)++] = argumentos->v->tipoDef;
    }
}
