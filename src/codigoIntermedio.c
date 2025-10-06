#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codigoIntermedio.h"

static void generarSentencia(CodigoIntermedio* generador, Nodo* n);
static char* generarExpr(CodigoIntermedio* generador, Nodo* n);
static char* nuevaTemp(CodigoIntermedio* generador);
static char* nuevoLabel(CodigoIntermedio* generador);
static void crearInstr(CodigoIntermedio* generador, TipoInstr tipo, AstValor arg1, AstValor arg2, AstValor res, AstValor label);

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

void imprimirCodigoIntermedio(CodigoIntermedio* generador) {
    if (!generador || !generador->head) {
        printf("No hay código intermedio generado.\n");
        return;
    }

    printf("CÓDIGO INTERMEDIO:\n");

    Instr* actual = generador->head;
    while (actual) {
        switch (actual->tipo) {
            case INSTR_ASSIGN:
                printf("%s = %s\n", actual->res.s, actual->arg1.s);
                break;

            case INSTR_ADD:
                printf("%s = %s + %s\n", actual->res.s, actual->arg1.s, actual->arg2.s);
                break;

            case INSTR_SUB:
                printf("%s = %s - %s\n", actual->res.s, actual->arg1.s, actual->arg2.s);
                break;

            case INSTR_MUL:
                printf("%s = %s * %s\n", actual->res.s, actual->arg1.s, actual->arg2.s);
                break;

            case INSTR_DIV:
                printf("%s = %s / %s\n", actual->res.s, actual->arg1.s, actual->arg2.s);
                break;

            case INSTR_MOD:
                printf("%s = %s %% %s\n", actual->res.s, actual->arg1.s, actual->arg2.s);
                break;

            case INSTR_EQ:
                printf("%s = (%s == %s)\n", actual->res.s, actual->arg1.s, actual->arg2.s);
                break;

            case INSTR_LT:
                printf("%s = (%s < %s)\n", actual->res.s, actual->arg1.s, actual->arg2.s);
                break;

            case INSTR_GT:
                printf("%s = (%s > %s)\n", actual->res.s, actual->arg1.s, actual->arg2.s);
                break;

            case INSTR_AND:
                printf("%s = %s && %s\n", actual->res.s, actual->arg1.s, actual->arg2.s);
                break;

            case INSTR_OR:
                printf("%s = %s || %s\n", actual->res.s, actual->arg1.s, actual->arg2.s);
                break;

            case INSTR_NOT:
                printf("%s = !%s\n", actual->res.s, actual->arg1.s);
                break;

            case INSTR_PARAM:
                printf("param %s\n", actual->arg1.s);
                break;

            case INSTR_CALL:
                printf("%s = call %s\n", actual->res.s, actual->arg1.s);
                break;

            case INSTR_LABEL:
                printf("%s:\n", actual->label.s);
                break;

            case INSTR_GOTO:
                printf("goto %s\n", actual->label.s);
                break;

            case INSTR_IF:
                printf("if %s goto %s\n", actual->arg1.s, actual->label.s);
                break;

            case INSTR_RETURN:
                if (actual->arg1.s)
                    printf("return %s\n", actual->arg1.s);
                else
                    printf("return\n");
                break;
            case INSTR_WHILE:
                printf("while %s goto %s\n", actual->arg1.s, actual->res.s); // arg1=condición, res=label de fin
                break;

            case INSTR_FUNC_BEGIN:
                printf("func_begin %s\n", actual->res.s);
                break;

            case INSTR_FUNC_END:
                printf("func_end %s\n", actual->res.s);
                break;

            case INSTR_RETVAL:
                printf("retval %s\n", actual->arg1.s);
                break;

            case INSTR_VAR_GLOBAL:
                printf("var_global %s\n", actual->res.s);
                break;

            case INSTR_VAR_LOCAL:
                printf("var_local %s\n", actual->res.s);
                break;

            default:
                printf("Instrucción desconocida (%d)\n", actual->tipo);
                break;
        }

        actual = actual->next;
    }

    printf("FIN DEL CÓDIGO INTERMEDIO\n");
}



// Genera el codigo intermedio para funciones/metodos
static void generarFuncion(CodigoIntermedio* generador, Nodo* n) {
    if (!n || n->tipo != AST_DECL_FUNC) return;

    AstValor nombre = *n->v;
    crearInstr(generador, INSTR_FUNC_BEGIN, (AstValor){0}, (AstValor){0}, nombre, (AstValor){0});

    generarSentencia(generador, n->hd);

    crearInstr(generador, INSTR_FUNC_END, (AstValor){0}, (AstValor){0}, nombre, (AstValor){0});
}

// Genera el codigo intermedio para declaraciones de variables globales o formales
static void generarDeclaracion(CodigoIntermedio* generador, Nodo* n, int esGlobal) {
    if (!n || n->tipo != AST_DECL_VAR) return;

    AstValor tipoVar = *n->v;

    if (esGlobal)
        crearInstr(generador, INSTR_VAR_GLOBAL, (AstValor){0}, (AstValor){0}, tipoVar, (AstValor){0});
    else
        crearInstr(generador, INSTR_VAR_LOCAL, (AstValor){0}, (AstValor){0}, tipoVar, (AstValor){0});

    if (n->hd) {
        char* expr = generarExpr(generador, n->hd);
        AstValor valorExpr;
        valorExpr.s = expr;
        valorExpr.tipoDef = n->v->tipoDef;
        crearInstr(generador, INSTR_ASSIGN, valorExpr, (AstValor){0}, *n->v, (AstValor){0});
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
            generarDeclaracion(generador, n, 1);
            break;

        case AST_ASIGNACION: {
            char* aux = generarExpr(generador, n->hd);
            AstValor valorAux;
            valorAux.s = aux;
            if (n->v != NULL) {
                valorAux.tipoDef = n->v->tipoDef;
            } else {
                valorAux.tipoDef = INT;
            }
            crearInstr(generador, INSTR_ASSIGN, valorAux, (AstValor){0}, *n->hi->v, (AstValor){0});
            free(aux);
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

            AstValor condValor;
            condValor.s = cond;
            condValor.tipoDef = BOOL;

            AstValor elseValor;
            elseValor.s = charElse;

            AstValor finValor;
            finValor.s = finIf;

            crearInstr(generador, INSTR_IF, condValor, (AstValor){0},(AstValor){0} , elseValor);
            generarSentencia(generador, n->hd);
            crearInstr(generador, INSTR_GOTO, (AstValor){0}, (AstValor){0}, (AstValor){0}, finValor);
            crearInstr(generador, INSTR_LABEL, (AstValor){0}, (AstValor){0}, (AstValor){0}, elseValor);
            if (n->extra) generarSentencia(generador, n->extra);
            crearInstr(generador, INSTR_LABEL, (AstValor){0}, (AstValor){0}, (AstValor){0}, finValor);

            free(cond);
            free(charElse);
            free(finIf);
            break;
        }

        case AST_WHILE: {
            char* comienzoWhile = nuevoLabel(generador);
            char* finWhile = nuevoLabel(generador);

            AstValor labelInicio;
            labelInicio.s = comienzoWhile;

            AstValor labelFin;
            labelFin.s = finWhile;

            crearInstr(generador, INSTR_LABEL, (AstValor){0}, (AstValor){0}, (AstValor){0}, labelInicio);
            char* cond = generarExpr(generador, n->hi);

            AstValor condValor;
            condValor.s = cond;
            condValor.tipoDef = BOOL;

            crearInstr(generador, INSTR_WHILE, condValor, (AstValor){0}, labelFin, (AstValor){0});
            generarSentencia(generador, n->hd);
            crearInstr(generador, INSTR_GOTO, (AstValor){0}, (AstValor){0}, labelInicio, (AstValor){0});
            crearInstr(generador, INSTR_LABEL, (AstValor){0}, (AstValor){0}, (AstValor){0}, labelFin);

            free(comienzoWhile);
            free(finWhile);
            free(cond);
            break;
        }

        case AST_RETURN: {
            if (n->hi) {
                char* valor = generarExpr(generador, n->hi);
                AstValor valAst;
                valAst.s = valor;
                if (n->v != NULL) {
                    valAst.tipoDef = n->v->tipoDef;
                } else {
                    valAst.tipoDef = INT;
                }
                crearInstr(generador, INSTR_RETURN, valAst, (AstValor){0}, (AstValor){0}, (AstValor){0});
                free(valor);
            } else {
                crearInstr(generador, INSTR_RETURN, (AstValor){0}, (AstValor){0}, (AstValor){0}, (AstValor){0});
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

static char* generarExpr(CodigoIntermedio* generador, Nodo* n) {
    if (!n) return NULL;

    switch (n->tipo) {

        case AST_INT: {
            char buffer[20];
            sprintf(buffer, "%ld", n->v->i);

            char* temp = nuevaTemp(generador);

            AstValor cte = { .s = strdup(buffer), .tipoDef = INT };
            AstValor res = { .s = strdup(temp), .tipoDef = INT };

            crearInstr(generador, INSTR_ASSIGN, cte, (AstValor){0}, res, (AstValor){0});

            free(cte.s);
            free(res.s);
            return temp;
        }

        case AST_BOOL: {
            char buffer[6];
            sprintf(buffer, "%d", n->v->b);

            char* temp = nuevaTemp(generador);

            AstValor cte = { .s = strdup(buffer), .tipoDef = BOOL };
            AstValor res = { .s = strdup(temp), .tipoDef = BOOL };

            crearInstr(generador, INSTR_ASSIGN, cte, (AstValor){0}, res, (AstValor){0});

            free(cte.s);
            free(res.s);
            return temp;
        }

        case AST_ID: {
            char* temp = nuevaTemp(generador);

            AstValor var = { .s = strdup(n->v->s), .tipoDef = n->v->tipoDef };
            AstValor res = { .s = strdup(temp), .tipoDef = n->v->tipoDef };

            crearInstr(generador, INSTR_ASSIGN, var, (AstValor){0}, res, (AstValor){0});

            free(var.s);
            free(res.s);
            return temp;
        }

        case AST_OP: {
            TipoOp op = n->v->op;

            if (op == OP_NOT) {
                char* opnd = generarExpr(generador, n->hi);
                char* temp = nuevaTemp(generador);

                AstValor a1 = { .s = strdup(opnd), .tipoDef = n->v->tipoDef };
                AstValor res = { .s = strdup(temp), .tipoDef = n->v->tipoDef };

                crearInstr(generador, INSTR_NOT, a1, (AstValor){0}, res, (AstValor){0});

                free(a1.s);
                free(res.s);
                free(opnd);
                return temp;
            }

            char* izq = generarExpr(generador, n->hi);
            char* der = generarExpr(generador, n->hd);
            char* temp = nuevaTemp(generador);

            AstValor a1 = { .s = strdup(izq), .tipoDef = n->hi->v->tipoDef };
            AstValor a2 = { .s = strdup(der), .tipoDef = n->hd->v->tipoDef };
            AstValor res = { .s = strdup(temp), .tipoDef = n->v->tipoDef };

            switch (op) {
                case OP_SUMA:  crearInstr(generador, INSTR_ADD, a1, a2, res, (AstValor){0}); break;
                case OP_RESTA: crearInstr(generador, INSTR_SUB, a1, a2, res, (AstValor){0}); break;
                case OP_MULT:  crearInstr(generador, INSTR_MUL, a1, a2, res, (AstValor){0}); break;
                case OP_DIV:   crearInstr(generador, INSTR_DIV, a1, a2, res, (AstValor){0}); break;
                case OP_MOD:   crearInstr(generador, INSTR_MOD, a1, a2, res, (AstValor){0}); break;
                case OP_IGUAL: crearInstr(generador, INSTR_EQ,  a1, a2, res, (AstValor){0}); break;
                case OP_MAYOR: crearInstr(generador, INSTR_GT,  a1, a2, res, (AstValor){0}); break;
                case OP_MENOR: crearInstr(generador, INSTR_LT,  a1, a2, res, (AstValor){0}); break;
                case OP_AND:   crearInstr(generador, INSTR_AND, a1, a2, res, (AstValor){0}); break;
                case OP_OR:    crearInstr(generador, INSTR_OR,  a1, a2, res, (AstValor){0}); break;
                default: break;
            }

            free(a1.s); free(a2.s); free(res.s);
            free(izq);  free(der);
            return temp;
        }

        case AST_LLAMADA: {
            Nodo* param = n->hd;
            while (param) {
                char* valor = generarExpr(generador, param);
                AstValor arg = { .s = strdup(valor) };
                crearInstr(generador, INSTR_PARAM, arg, (AstValor){0}, (AstValor){0}, (AstValor){0});
                free(arg.s);
                free(valor);
                param = param->hd;
            }

            AstValor func = { .s = strdup(n->hi->v->s) };
            char* temp = nuevaTemp(generador);
            AstValor res = { .s = strdup(temp) };

            crearInstr(generador, INSTR_CALL, func, (AstValor){0}, res, (AstValor){0});

            free(func.s);
            free(res.s);
            return temp;
        }

        default:
            fprintf(stderr, "generarExpr: tipo de nodo no soportado (%d)\n", n->tipo);
            return strdup("0");
    }
}



// Crea un nuevo temporal (t0, t1, ..., tn)
static char* nuevaTemp(CodigoIntermedio* generador) {
    char buffer[20];
    sprintf(buffer, "t%d", generador->tempCount++);
    return strdup(buffer);
}

// Crea un nuevo label (L0, L1, ..., Ln)
static char* nuevoLabel(CodigoIntermedio* generador) {
    char buffer[20];
    sprintf(buffer, "L%d", generador->labelCount++);
    return strdup(buffer);
}

// Retorna el string del tipo que tiene la instruccion
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

// Funcion auxiliar para crear una nueva instruccion
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
