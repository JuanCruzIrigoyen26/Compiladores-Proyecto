#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "codigoObjeto.h"
#include "ts.h"

static int esConstante(const char* s);
static int esMem(const char* s);
static int esRegistro(const char* s);
static const char* registroParametro(const char* nombre);
static void moverARegistro(const char* reg, const char* src, FILE* out);
static void cargarEnRAX(const char* op, FILE* out);
static void cargarEnRBX(const char* op, FILE* out);
static void guardarDesdeRAX(const char* dst, FILE* out);
static void asignarTraduccion(FILE* out, const char* dst, const char* src);

static int paramIndex = 0; // Cuenta cuantos PARAM vimos antes de un CALL
static int paramStackCount = 0; // Cuantos PARAM fueron pasados por stack
static int localCount = 0; // Cantidad de variables locales de una funcion


// Función auxiliar para traducir operaciones aritméticas
static void traducirOperacion(Instr* instr, FILE* out) {
    if (!instr) return;

    switch (instr->tipo) {
        case INSTR_ADD:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    ADD rax, rbx\n");
            guardarDesdeRAX(instr->res.s, out);
            break;

        case INSTR_SUB:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    SUB rax, rbx\n");
            guardarDesdeRAX(instr->res.s, out);
            break;

        case INSTR_MUL:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    IMUL rax, rbx\n");
            guardarDesdeRAX(instr->res.s, out);
            break;

        case INSTR_DIV:
            cargarEnRAX(instr->arg1.s, out);
            fprintf(out, "    CQO\n");
            if (esConstante(instr->arg2.s) || !instr->arg2.s) {
                cargarEnRBX(instr->arg2.s ? instr->arg2.s : "1", out);
                fprintf(out, "    IDIV rbx\n");
            } else if (esMem(instr->arg2.s)) {
                fprintf(out, "    IDIV qword %s\n", instr->arg2.s);
            } else {
                fprintf(out, "    IDIV qword [%s]\n", instr->arg2.s);
            }
            guardarDesdeRAX(instr->res.s, out);
            break;
        case INSTR_MOD:
            cargarEnRAX(instr->arg1.s, out);
            fprintf(out, "    CQO\n");
            if (esConstante(instr->arg2.s) || !instr->arg2.s) {
                cargarEnRBX(instr->arg2.s ? instr->arg2.s : "1", out);
                fprintf(out, "    IDIV rbx\n");
            } else if (esMem(instr->arg2.s)) {
                fprintf(out, "    IDIV qword %s\n", instr->arg2.s);
            } else {
                fprintf(out, "    IDIV qword [%s]\n", instr->arg2.s);
            }
            fprintf(out, "    MOV rax, rdx\n");
            guardarDesdeRAX(instr->res.s, out);
            break;
        case INSTR_LT:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    CMP rax, rbx\n");
            fprintf(out, "    SETL al\n");
            fprintf(out, "    MOVZX rax, al\n");
            guardarDesdeRAX(instr->res.s, out);
            break;
        case INSTR_GT:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    CMP rax, rbx\n");
            fprintf(out, "    SETG al\n");
            fprintf(out, "    MOVZX rax, al\n");
            guardarDesdeRAX(instr->res.s, out);
            break;
        case INSTR_EQ:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    CMP rax, rbx\n");
            fprintf(out, "    SETE al\n");
            fprintf(out, "    MOVZX rax, al\n");
            guardarDesdeRAX(instr->res.s, out);
            break;
        case INSTR_AND:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    AND rax, rbx\n");
            guardarDesdeRAX(instr->res.s, out);
            break;
        case INSTR_OR:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    OR rax, rbx\n");
            guardarDesdeRAX(instr->res.s, out);
            break;
        case INSTR_NOT:
            cargarEnRAX(instr->arg1.s, out);
            fprintf(out, "    CMP rax, 0\n");
            fprintf(out, "    SETE al\n");
            fprintf(out, "    MOVZX rax, al\n");
            guardarDesdeRAX(instr->res.s, out);
            break;
        case INSTR_ASSIGN:
            asignarTraduccion(out, instr->res.s, instr->arg1.s);
            break;
        default:
            break;
    }
}

// Genera el código objeto en base al código intermedio
void generarCodigoObjeto(CodigoIntermedio* generador, FILE* out) {
    if (!generador || !generador->head) {
        fprintf(out, "; No hay código intermedio para generar\n");
        return;
    }

    int dataPrinted = 0;
    for (Instr* it = generador->head; it; it = it->next) {
        if (it->tipo == INSTR_VAR_GLOBAL && it->res.s) {
            if (!dataPrinted) {
                fprintf(out, "section .data\n");
                dataPrinted = 1;
            }
            fprintf(out, "%s dq 0\n", it->res.s);
        }
    }

    fprintf(out, "\nsection .text\n");
    fprintf(out, "global main\n\n");

    Instr* actual = generador->head;
    int funcReturn = 0;
    int esLabel = 0;
    while (actual) {
        switch (actual->tipo) {
            case INSTR_FUNC_BEGIN:
                localCount = 0;
                funcReturn = 0;
                esLabel = 0;
                paramIndex = 0;
                Instr* aux = actual->next;
                while (aux && aux->tipo != INSTR_FUNC_END) {
                    if (aux->tipo == INSTR_VAR_LOCAL)
                        localCount++;
                    aux = aux->next;
                }
                fprintf(out, "\n%s:\n", actual->res.s);
                fprintf(out, "    PUSH rbp\n");
                fprintf(out, "    MOV rbp, rsp\n");
                fprintf(out, "    SUB rsp, %d\n", localCount * 8);
                break;
            case INSTR_FUNC_END:
                if (!funcReturn) {
                    if (strcmp(actual->res.s, "main") == 0) {
                        fprintf(out, "    MOV rax, 0\n");
                    }
                    fprintf(out, "    MOV rsp, rbp\n");
                    fprintf(out, "    POP rbp\n");
                    fprintf(out, "    RET\n");
                }
                break;
            case INSTR_VAR_GLOBAL:
                break;
            case INSTR_ADD:
            case INSTR_SUB:
            case INSTR_MUL:
            case INSTR_DIV:
            case INSTR_MOD:
            case INSTR_ASSIGN:
            case INSTR_LT:
            case INSTR_GT:
            case INSTR_EQ:
            case INSTR_AND:
            case INSTR_OR:
            case INSTR_NOT:
                traducirOperacion(actual, out);
                esLabel = 0;
                break;
            case INSTR_PARAM: {
                const char* reg = regArg(paramIndex);
                if (reg) {
                    moverARegistro(reg, actual->arg1.s, out);
                } else {
                    cargarEnRAX(actual->arg1.s, out);
                    fprintf(out, "    PUSH rax\n");
                    paramStackCount++;
                }
                paramIndex++;
                esLabel = 0;
                break;
            }
            case INSTR_CALL:
                fprintf(out, "    CALL %s\n", actual->arg1.s);
                if (paramStackCount > 0) {
                    fprintf(out, "    ADD rsp, %d\n", paramStackCount * 8);
                }
                paramIndex = 0;
                paramStackCount = 0;
                if (actual->res.s) {
                    guardarDesdeRAX(actual->res.s, out);
                }
                esLabel = 0;
                break;
            case INSTR_RETURN:
                if (actual->arg1.s) {
                    cargarEnRAX(actual->arg1.s, out);
                }
                fprintf(out, "    MOV rsp, rbp\n");
                fprintf(out, "    POP rbp\n");
                fprintf(out, "    RET\n");
                funcReturn = 1;
                esLabel = 0;
                break;
            case INSTR_LABEL:
                fprintf(out, "%s:\n", actual->label.s);
                esLabel = 1;
                break;
            case INSTR_GOTO:
                fprintf(out, "    JMP %s\n", actual->label.s);
                esLabel = 0;
                break;
            case INSTR_IF:
                if (actual->arg1.s) {
                    if (esConstante(actual->arg1.s)) {
                        fprintf(out, "    MOV rax, %s\n", actual->arg1.s);
                        fprintf(out, "    CMP rax, 0\n");
                    } else if (esMem(actual->arg1.s)) {
                        fprintf(out, "    CMP qword %s, 0\n", actual->arg1.s);
                    } else {
                        fprintf(out, "    CMP qword [%s], 0\n", actual->arg1.s);
                    }
                } else {
                    fprintf(out, "    CMP rax, 0\n");
                }
                fprintf(out, "    JE %s\n", actual->label.s);
                esLabel = 0;
                break;
            case INSTR_WHILE:
                if (actual->arg1.s) {
                    if (esConstante(actual->arg1.s)) {
                        fprintf(out, "    MOV rax, %s\n", actual->arg1.s);
                        fprintf(out, "    CMP rax, 0\n");
                    } else if (esMem(actual->arg1.s)) {
                        fprintf(out, "    CMP qword %s, 0\n", actual->arg1.s);
                    } else {
                        fprintf(out, "    CMP qword [%s], 0\n", actual->arg1.s);
                    }
                } else {
                    fprintf(out, "    CMP rax, 0\n");
                }
                fprintf(out, "    JE %s\n", actual->label.s);
                esLabel = 0;
                break;
            case INSTR_VAR_LOCAL:
                localCount++;
            esLabel = 0;
            break;
            default:
                fprintf(out, "    ; Instrucción no implementada (%d)\n", actual->tipo);
                break;
        }

        actual = actual->next;
    }
}

// Devuelve el registro si el identificador es uno de los parámetros de la función
static const char* registroParametro(const char* nombre) {
    Simbolo* s = buscarSimbolo((char*)nombre);
    if (!s || !s->v) return NULL;

    if (s->v->esParametro && s->v->offset >= 0 && s->v->offset < 6) {
        return regArg(s->v->offset);
    }

    return NULL;
}

// Devuelve el registro que le corresponde al indice del parámetro en una llamada
const char* regArg(int index) {
    switch(index) {
        case 0: return "rdi";
        case 1: return "rsi";
        case 2: return "rdx";
        case 3: return "rcx";
        case 4: return "r8";
        case 5: return "r9";
        default: return NULL;
    }
}

// Mueve una constante, memoria o identificador a un registro
static void moverARegistro(const char* reg, const char* src, FILE* out) {
    if (!reg || !src) return;
    if (esConstante(src)) {
        fprintf(out, "    MOV %s, %s\n", reg, src);
    } else if (esMem(src)) {
        fprintf(out, "    MOV %s, %s\n", reg, src);
    } else {
        fprintf(out, "    MOV %s, [%s]\n", reg, src);
    }
}

// Chequea si una cadena representa a una constante
static int esConstante(const char* s) {
    if (!s || !*s) return 0;
    const char* p = s;
    if (*p=='+' || *p=='-') ++p;
    if (!*p) return 0;
    for (; *p; ++p) if (!isdigit((unsigned char)*p)) return 0;
    return 1;
}

// Chequea si la cadena representa una dirección de memoria
static int esMem(const char* s) {
    if (!s) return 0;
    while (*s==' ' || *s=='\t') ++s;
    return *s=='[';
}

// Chequea si la cadena representa un registro
static int esRegistro(const char* s) {
    return (
        strcmp(s, "rdi")==0 ||
        strcmp(s, "rsi")==0 ||
        strcmp(s, "rdx")==0 ||
        strcmp(s, "rcx")==0 ||
        strcmp(s, "r8")==0  ||
        strcmp(s, "r9")==0  ||
        strcmp(s, "rax")==0 ||
        strcmp(s, "rbx")==0
    );
}

// Carga el valor de una expresión en el registro rax
static void cargarEnRAX(const char* op, FILE* out) {
    if (!op) return;

    if (esRegistro(op)) {
        fprintf(out, "    MOV rax, %s\n", op);
        return;
    }

    const char* reg = registroParametro(op);
    if (reg) {
        fprintf(out, "    MOV rax, %s\n", reg);
        return;
    }

    if (esConstante(op)) {
        fprintf(out, "    MOV rax, %s\n", op);
    } else if (esMem(op)) {
        fprintf(out, "    MOV rax, %s\n", op);
    } else {
        fprintf(out, "    MOV rax, [%s]\n", op);
    }
}


// Carga el valor de una expresión en el registro rbx
static void cargarEnRBX(const char* op, FILE* out) {
    if (!op) return;

    if (esRegistro(op)) {
        fprintf(out, "    MOV rbx, %s\n", op);
        return;
    }

    const char* reg = registroParametro(op);
    if (reg) {
        fprintf(out, "    MOV rbx, %s\n", reg);
        return;
    }

    if (esConstante(op)) {
        fprintf(out, "    MOV rbx, %s\n", op);
    } else if (esMem(op)) {
        fprintf(out, "    MOV rbx, %s\n", op);
    } else {
        fprintf(out, "    MOV rbx, [%s]\n", op);
    }
}


// Guarda lo que contiene rax en la dirección de destino
static void guardarDesdeRAX(const char* dst, FILE* out) {
    if (!dst) { fprintf(out, "    ; guardarDesdeRAX(dst=NULL)\n"); return; }
    if (esMem(dst)) {
        fprintf(out, "    MOV %s, rax\n", dst);
    } else {
        fprintf(out, "    MOV [%s], rax\n", dst);
    }
}

// Funcion auxiliar para traducir la operación de asignación de código intermedio a código objeto
static void asignarTraduccion(FILE* out, const char* dst, const char* src) {
    if (!dst || !src) {
        fprintf(out, "    ; asignar(dst/src NULL)\n");
        return;
    }
    if (esConstante(src)) {
        if (esMem(dst)) fprintf(out, "    MOV %s, %s\n", dst, src);
        else            fprintf(out, "    MOV [%s], %s\n", dst, src);
    } else {
        cargarEnRAX(src, out);
        guardarDesdeRAX(dst, out);
    }
}