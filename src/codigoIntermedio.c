#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codigoIntermedio.h"
#include "ts.h"
#include "codigoObjeto.h"

static void generarSentencia(CodigoIntermedio* generador, Nodo* n);
static char* generarExpr(CodigoIntermedio* generador, Nodo* n);
static char* nuevaTemp(CodigoIntermedio* generador);
static char* nuevoLabel(CodigoIntermedio* generador);
static void crearInstr(CodigoIntermedio* generador, TipoInstr tipo, AstValor arg1, AstValor arg2, AstValor res, AstValor label);
static const char* instrToStr(TipoInstr tipo);
static void asignarOffset(AstValor* v);
static char* convertirReferencia(AstValor v);

static int offsetLocal = 0;
static int enFuncion = 0;
static tipoDef tipoFuncionActual = VOID;

// Crea el generador de codigo intermedio
CodigoIntermedio* crearGenerador() {
    CodigoIntermedio* generador = malloc(sizeof(CodigoIntermedio));
    generador->head = generador->tail = NULL;
    generador->tempCount = 0;
    generador->labelCount = 0;
    return generador;
}

// Funcion principal de generacion del codigo intermedio
void generarCodigoIntermedio(CodigoIntermedio* generador, Nodo* raiz) {
    generarSentencia(generador, raiz);
}

// Imprime en pantalla el codigo de tres direcciones (codigo intermedio)
void imprimirCodigoIntermedio(FILE* out, CodigoIntermedio* generador) {
    if (!generador || !generador->head) {
        fprintf(out, "No hay código intermedio generado.\n");
        return;
    }

    fprintf(out, "%-12s %-12s %-12s %-12s %-12s\n", 
           "OP", "ARG1", "ARG2", "RES", "LABEL");

    Instr* actual = generador->head;
    while (actual) {
        const char* op    = instrToStr(actual->tipo);
        const char* arg1  = (actual->arg1.s != NULL) ? actual->arg1.s : "_";
        const char* arg2  = (actual->arg2.s != NULL) ? actual->arg2.s : "_";
        const char* res   = (actual->res.s  != NULL) ? actual->res.s  : "_";
        const char* label = (actual->label.s != NULL) ? actual->label.s : "_";

        fprintf(out, "%-12s %-12s %-12s %-12s %-12s\n", op, arg1, arg2, res, label);
        actual = actual->next;
    }

    fprintf(out, "\n");
}

// Genera el codigo intermedio para funciones/metodos
static void generarFuncion(CodigoIntermedio* generador, Nodo* n) {
    enFuncion = 1;
    offsetLocal = 0;
    tipoFuncionActual = n->v ? n->v->tipoDef : VOID;

    prepararParametros();
    insertarParametros(n->hi);

    AstValor nombre = *n->v;
    crearInstr(generador, INSTR_FUNC_BEGIN, (AstValor){0}, (AstValor){0}, nombre, (AstValor){0});

    generarSentencia(generador, n->hd);

    crearInstr(generador, INSTR_FUNC_END, (AstValor){0}, (AstValor){0}, nombre, (AstValor){0});
    enFuncion = 0;
    tipoFuncionActual = VOID;
}

// Genera el codigo intermedio para declaraciones de variables globales o formales
static void generarDeclaracion(CodigoIntermedio* generador, Nodo* n, int esGlobal) {
    Simbolo* sim = buscarSimbolo(n->v->s);
    AstValor* v = sim ? sim->v : n->v;
    esGlobal = !enFuncion;

    if (esGlobal) {
        v->offset = 0;
        crearInstr(generador, INSTR_VAR_GLOBAL, (AstValor){0}, (AstValor){0}, *v, (AstValor){0});
    } else {
        asignarOffset(v);
        AstValor valor = *v;
        valor.s = convertirReferencia(*v);
        crearInstr(generador, INSTR_VAR_LOCAL, (AstValor){0}, (AstValor){0}, valor, (AstValor){0});
    }

    if (n->hd) {
        char* expr = generarExpr(generador, n->hd);
        AstValor valorExpr = { .s = expr, .tipoDef = v->tipoDef };
        AstValor ref = *v;
        ref.s = convertirReferencia(*v);
        crearInstr(generador, INSTR_ASSIGN, valorExpr, (AstValor){0}, ref, (AstValor){0});
        free(expr);
    }
}

// Genera el codigo intermedio para sentencias
static void generarSentencia(CodigoIntermedio* generador, Nodo* n) {
    if (!n) return;

    switch (n->tipo) {
        case AST_DECL_FUNC:
            generarFuncion(generador, n);
            break;

        case AST_DECL_VAR:
            generarDeclaracion(generador, n, 0);
            break;

        case AST_ASIGNACION: {
            char* aux = generarExpr(generador, n->hd);
            AstValor valorAux = { .s = aux, .tipoDef = n->hi->v->tipoDef };
            AstValor ladoIzq = *n->hi->v;
            char* ref = convertirReferencia(ladoIzq);
            ladoIzq.s = ref;
            crearInstr(generador, INSTR_ASSIGN, valorAux, (AstValor){0}, ladoIzq, (AstValor){0});
            free(aux);
            free(ref);
            break;
        }
        case AST_LLAMADA: {
            generarExpr(generador, n);
            break;
        }

        case AST_STMTS:
        case AST_DECLS:
        case AST_BLOQUE:
            generarSentencia(generador, n->hi);
            generarSentencia(generador, n->hd);
            break;

        case AST_IF: {
            char* cond = generarExpr(generador, n->hi);
            char* charElse = nuevoLabel(generador);
            char* finIf = nuevoLabel(generador);

            AstValor condValor = { .s = cond, .tipoDef = BOOL };
            AstValor elseValor = { .s = charElse };
            AstValor finValor = { .s = finIf };

            crearInstr(generador, INSTR_IF, condValor, (AstValor){0}, (AstValor){0}, elseValor);
            generarSentencia(generador, n->hd);
            crearInstr(generador, INSTR_GOTO, (AstValor){0}, (AstValor){0}, (AstValor){0}, finValor);
            crearInstr(generador, INSTR_LABEL, (AstValor){0}, (AstValor){0}, (AstValor){0}, elseValor);
            if (n->extra) generarSentencia(generador, n->extra);
            crearInstr(generador, INSTR_LABEL, (AstValor){0}, (AstValor){0}, (AstValor){0}, finValor);

            free(cond); free(charElse); free(finIf);
            break;
        }
        case AST_WHILE: {
            char* comienzoWhile = nuevoLabel(generador);
            char* finWhile = nuevoLabel(generador);

            AstValor labelInicio = { .s = comienzoWhile };
            AstValor labelFin = { .s = finWhile };

            crearInstr(generador, INSTR_LABEL, (AstValor){0}, (AstValor){0}, (AstValor){0}, labelInicio);
            char* cond = generarExpr(generador, n->hi);

            AstValor condValor = { .s = cond, .tipoDef = BOOL };
            crearInstr(generador, INSTR_WHILE, condValor, (AstValor){0}, (AstValor){0}, labelFin);
            generarSentencia(generador, n->hd);
            crearInstr(generador, INSTR_GOTO, (AstValor){0}, (AstValor){0}, (AstValor){0}, labelInicio);
            crearInstr(generador, INSTR_LABEL, (AstValor){0}, (AstValor){0}, (AstValor){0}, labelFin);

            free(comienzoWhile); free(finWhile); free(cond);
            break;
        }
        case AST_RETURN: {
            if (tipoFuncionActual == VOID) {
                if (n->hi) generarExpr(generador, n->hi);
                crearInstr(generador, INSTR_RETURN, (AstValor){0}, (AstValor){0}, (AstValor){0}, (AstValor){0});
            } else {
                char* valor = generarExpr(generador, n->hi);
                AstValor valAst = { .s = valor };
                crearInstr(generador, INSTR_RETURN, valAst, (AstValor){0}, (AstValor){0}, (AstValor){0});
                free(valor);
            }
            break;
        }

        default:
            generarSentencia(generador, n->hi);
            generarSentencia(generador, n->hd);
            generarSentencia(generador, n->extra);
            break;
    }
}

// Genera el codigo intermedio para expresiones
static char* generarExpr(CodigoIntermedio* generador, Nodo* n) {
    if (!n) return NULL;

    switch (n->tipo) {
        case AST_INT: {
            char buffer[20];
            sprintf(buffer, "%ld", n->v->i);
            return strdup(buffer);
        }

        case AST_BOOL: {
            return strdup(n->v->b ? "1" : "0");
        }

        case AST_ID: {
            return convertirReferencia(*n->v);
        }

        case AST_OP: {
            TipoOp op = n->v->op;
            if (op == OP_NOT) {
                char* opnd = generarExpr(generador, n->hi);
                char* temp = nuevaTemp(generador);
                crearInstr(generador, INSTR_NOT, (AstValor){.s=opnd}, (AstValor){0}, (AstValor){.s=temp}, (AstValor){0});
                free(opnd);
                return temp;
            }

            char* izq = generarExpr(generador, n->hi);
            char* der = generarExpr(generador, n->hd);
            char* temp = nuevaTemp(generador);

            AstValor a1 = { .s = izq };
            AstValor a2 = { .s = der };
            AstValor res = { .s = temp };

            switch (op) {
                case OP_SUMA: crearInstr(generador, INSTR_ADD, a1, a2, res, (AstValor){0}); break;
                case OP_RESTA: crearInstr(generador, INSTR_SUB, a1, a2, res, (AstValor){0}); break;
                case OP_MULT: crearInstr(generador, INSTR_MUL, a1, a2, res, (AstValor){0}); break;
                case OP_DIV:  crearInstr(generador, INSTR_DIV, a1, a2, res, (AstValor){0}); break;
                case OP_MOD:  crearInstr(generador, INSTR_MOD, a1, a2, res, (AstValor){0}); break;
                case OP_IGUAL: crearInstr(generador, INSTR_EQ, a1, a2, res, (AstValor){0}); break;
                case OP_MAYOR: crearInstr(generador, INSTR_GT, a1, a2, res, (AstValor){0}); break;
                case OP_MENOR: crearInstr(generador, INSTR_LT, a1, a2, res, (AstValor){0}); break;
                case OP_AND:  crearInstr(generador, INSTR_AND, a1, a2, res, (AstValor){0}); break;
                case OP_OR:   crearInstr(generador, INSTR_OR, a1, a2, res, (AstValor){0}); break;
                default: break;
            }

            free(izq); free(der);
            return temp;
        }

        case AST_LLAMADA: {
            char **vals = NULL;
            size_t nvals = 0, cap = 0;

            Nodo* p = n->hd;
            while (p) {
                Nodo* exprNodo = (p->tipo == AST_SEQ_EXPR) ? p->hi : p;

                // evalúa la expresión del parámetro (puede generar CALLs internas)
                char* valor = generarExpr(generador, exprNodo);

                if (nvals == cap) {
                    cap = cap ? cap * 2 : 4;
                    vals = (char**)realloc(vals, cap * sizeof(char*));
                }
                vals[nvals++] = valor;

                if (p->tipo == AST_SEQ_EXPR) p = p->hd;
                else break;
            }
            for (size_t i = 0; i < nvals; ++i) {
                crearInstr(generador,
                           INSTR_PARAM,
                           (AstValor){ .s = vals[i] },
                           (AstValor){0},
                           (AstValor){0},
                           (AstValor){0});
            }
            for (size_t i = 0; i < nvals; ++i) {
                free(vals[i]);
            }
            free(vals);
            char* temp = nuevaTemp(generador);
            crearInstr(generador, INSTR_CALL, (AstValor){ .s = n->hi->v->s }, (AstValor){0}, (AstValor){ .s = temp }, (AstValor){0});
            return temp;
        }

        default:
            fprintf(stderr, "generarExpr: tipo de nodo no soportado (%d)\n", n->tipo);
            return strdup("0");
    }
}


// Crea un nuevo temporal y le asigna un offset en el stack (rbp-8, rbp-16, ...)
static char* nuevaTemp(CodigoIntermedio* generador) {
    AstValor temp = {0};
    asignarOffset(&temp);
    char* ref = malloc(20);
    sprintf(ref, "[rbp-%d]", temp.offset);
    temp.s = ref;
    crearInstr(generador, INSTR_VAR_LOCAL, (AstValor){0}, (AstValor){0}, temp, (AstValor){0});
    return ref;
}

// Crea un nuevo label (L0, L1, ..., Ln)
static char* nuevoLabel(CodigoIntermedio* generador) {
    char buffer[20];
    sprintf(buffer, "L%d", generador->labelCount++);
    return strdup(buffer);
}

// Retorna el string del tipo que tiene la actual
static const char* instrToStr(TipoInstr tipo) {
    switch (tipo) {
        case INSTR_ADD: return "ADD";
        case INSTR_SUB: return "SUB";
        case INSTR_MUL: return "MUL";
        case INSTR_DIV: return "DIV";
        case INSTR_MOD: return "MOD";
        case INSTR_LT:  return "LT";
        case INSTR_GT:  return "GT";
        case INSTR_EQ:  return "EQ";
        case INSTR_AND: return "AND";
        case INSTR_OR:  return "OR";
        case INSTR_NOT: return "NOT";
        case INSTR_ASSIGN: return "ASSIGN";
        case INSTR_IF:    return "IF";
        case INSTR_WHILE: return "WHILE";
        case INSTR_GOTO:     return "GOTO";
        case INSTR_LABEL:    return "LABEL";
        case INSTR_FUNC_BEGIN: return "FUNC_BEGIN";
        case INSTR_FUNC_END:   return "FUNC_END";
        case INSTR_PARAM:    return "PARAM";
        case INSTR_CALL:     return "CALL";
        case INSTR_RETVAL:   return "RETVAL";
        case INSTR_RETURN:   return "RETURN";
        case INSTR_VAR_GLOBAL: return "VAR_GLOBAL";
        case INSTR_VAR_LOCAL:  return "VAR_LOCAL";
        default: return "UNKNOWN";
    }
}

// Funcion auxiliar para crear una nueva actual
static void crearInstr(CodigoIntermedio* generador, TipoInstr tipo, AstValor arg1, AstValor arg2, AstValor res, AstValor label) {
    Instr* instr = malloc(sizeof(Instr));
    instr->tipo = tipo;

    instr->arg1 = arg1;
    if (arg1.s != NULL) {
        instr->arg1.s = strdup(arg1.s);
    } else {
        instr->arg1.s = NULL;
    }

    instr->arg2 = arg2;
    if (arg2.s != NULL) {
        instr->arg2.s = strdup(arg2.s);
    } else {
        instr->arg2.s = NULL;
    }

    instr->res = res;
    if (res.s != NULL) {
        instr->res.s = strdup(res.s);
    } else {
        instr->res.s = NULL;
    }

    instr->label = label;
    if (label.s != NULL) {
        instr->label.s = strdup(label.s);
    } else {
        instr->label.s = NULL;
    }

    instr->next = NULL;

    if (generador->head == NULL) {
        generador->head = instr;
    } else {
        generador->tail->next = instr;
    }
    generador->tail = instr;
}

// Asigna un offset en el stack para una variable local
static void asignarOffset(AstValor* v) {
    offsetLocal += 8;
    v->offset = offsetLocal;
}

// Convierte un simbolo a la referencia que le corresponde en codigo objeto
static char* convertirReferencia(AstValor v) {
    if (!v.esFuncion && !v.esParametro && v.offset == 0) {
        return strdup(v.s);
    }

    if (v.esParametro && v.offset >= 0 && v.offset < 6) {
        return strdup(regArg(v.offset));
    }

    if (v.offset > 0) {
        char* aux = malloc(20);
        sprintf(aux, "[rbp-%d]", v.offset);
        return aux;
    }

    return strdup(v.s);
}