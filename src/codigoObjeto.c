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
static const char* attAddr(const char* op); 


static int paramIndex = 0; // Cuenta cuantos PARAM vimos antes de un CALL
static int paramStackCount = 0; // Cuantos PARAM fueron pasados por stack
static int localCount = 0; // Cantidad de variables locales de una funcion

//Funcion para traducir la sintaxis de memoria [rbp-8] o [var] a -8(%rbp) o var
static char att_mem_buf[256];
static const char* attAddr(const char* op) {
    if (!op) return "NULL";
    
    // Si es [memoria]
    if (esMem(op)) { 
        const char* p = op + 1; // Saltar '['
        char inner[250];
        char* end = strchr(p, ']');
        if (!end) return op; // Mal formado

        size_t len = end - p;
        strncpy(inner, p, len);
        inner[len] = '\0';

        // Detectar [rbp-X] o [rbp+X]
        if (strncmp(inner, "rbp", 3) == 0) {
            long offset = 0;
            char* offset_str = inner + 3;
            if (*offset_str == '-' || *offset_str == '+') {
                offset = strtol(offset_str, NULL, 10);
            }
            // Formato AT&T: -8(%rbp) [cite: 993-1002, 1039-1048]
            sprintf(att_mem_buf, "%ld(%%rbp)", offset); 
            return att_mem_buf;
        }
        // Detectar [reg]
        if (esRegistro(inner)) {
            sprintf(att_mem_buf, "(%%%s)", inner); // Formato AT&T: (%rbx)
            return att_mem_buf;
        }
        
        // Asumir [var_global] -> var_global
        sprintf(att_mem_buf, "%s(%%rip)", inner);
        return att_mem_buf;
    } else {
        // Si es una variable global (sin corchetes), se referencia por nombre
        sprintf(att_mem_buf, "%s(%%rip)", op);
        return att_mem_buf;
    }
}


// Función auxiliar para traducir operaciones aritméticas
static void traducirOperacion(Instr* instr, FILE* out) {
    if (!instr) return;

    switch (instr->tipo) {
        case INSTR_ADD:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    add %%rbx, %%rax\n"); 
            guardarDesdeRAX(instr->res.s, out);
            break;

        case INSTR_SUB:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    sub %%rbx, %%rax\n"); 
            guardarDesdeRAX(instr->res.s, out);
            break;

        case INSTR_MUL:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    imul %%rbx, %%rax\n"); 
            guardarDesdeRAX(instr->res.s, out);
            break;

        case INSTR_DIV:
            cargarEnRAX(instr->arg1.s, out);
            fprintf(out, "    cqto\n"); 
            if (esConstante(instr->arg2.s) || !instr->arg2.s) {
                cargarEnRBX(instr->arg2.s ? instr->arg2.s : "1", out);
                fprintf(out, "    idiv %%rbx\n"); 
            }else { 
                fprintf(out, "    idiv %s\n", attAddr(instr->arg2.s));
            }
            guardarDesdeRAX(instr->res.s, out);
            break;
        case INSTR_MOD:
            cargarEnRAX(instr->arg1.s, out);
            fprintf(out, "    cqto\n");
            if (esConstante(instr->arg2.s) || !instr->arg2.s) {
                cargarEnRBX(instr->arg2.s ? instr->arg2.s : "1", out);
                fprintf(out, "    idiv %%rbx\n");
            } else {
                fprintf(out, "    idiv %s\n", attAddr(instr->arg2.s));
            }
            fprintf(out, "    mov %%rdx, %%rax\n"); 
            guardarDesdeRAX(instr->res.s, out);
            break;
        case INSTR_LT:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    cmp %%rbx, %%rax\n"); 
            fprintf(out, "    setl %%al\n"); 
            fprintf(out, "    movzbq %%al, %%rax\n"); 
            guardarDesdeRAX(instr->res.s, out);
            break;
        case INSTR_GT:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    cmp %%rbx, %%rax\n");
            fprintf(out, "    setg %%al\n"); 
            fprintf(out, "    movzbq %%al, %%rax\n");
            guardarDesdeRAX(instr->res.s, out);
            break;
        case INSTR_EQ:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    cmp %%rbx, %%rax\n");
            fprintf(out, "    sete %%al\n"); 
            fprintf(out, "    movzbq %%al, %%rax\n");
            guardarDesdeRAX(instr->res.s, out);
            break;
        case INSTR_AND:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    and %%rbx, %%rax\n");
            guardarDesdeRAX(instr->res.s, out);
            break;
        case INSTR_OR:
            cargarEnRAX(instr->arg1.s, out);
            cargarEnRBX(instr->arg2.s, out);
            fprintf(out, "    or %%rbx, %%rax\n");
            guardarDesdeRAX(instr->res.s, out);
            break;
        case INSTR_NOT:
            cargarEnRAX(instr->arg1.s, out);
            fprintf(out, "    cmp $0, %%rax\n"); 
            fprintf(out, "    sete %%al\n"); 
            fprintf(out, "    movzbq %%al, %%rax\n");
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
        fprintf(out, "# No hay código intermedio para generar\n"); 
        return;
    }

    int dataPrinted = 0;
    for (Instr* it = generador->head; it; it = it->next) {
        if (it->tipo == INSTR_VAR_GLOBAL && it->res.s) {
            if (!dataPrinted) {
                fprintf(out, ".data\n");
                dataPrinted = 1;
            }
            fprintf(out, "%s: .quad 0\n", it->res.s); 
        }
    }

    fprintf(out, "\n.text\n"); 
    fprintf(out, ".globl main\n\n"); 

    Instr* actual = generador->head;
    int funcReturn = 0;
    while (actual) {
        switch (actual->tipo) {
            case INSTR_FUNC_BEGIN:
                localCount = 0;
                funcReturn = 0;
                paramIndex = 0;
                paramStackCount = 0;
                Instr* aux = actual->next;
                while (aux && aux->tipo != INSTR_FUNC_END) {
                    if (aux->tipo == INSTR_VAR_LOCAL)
                        localCount++;
                    aux = aux->next;
                }
                fprintf(out, "\n%s:\n", actual->res.s); 
                fprintf(out, "    push %%rbp\n"); 
                fprintf(out, "    mov %%rsp, %%rbp\n"); 

                int bytesTemp = localCount * 8;
                int alineacion = (bytesTemp % 16 == 0) ? 8 : 0;
                fprintf(out, "    sub $%d, %%rsp\n", bytesTemp + alineacion); 
                break;
            case INSTR_FUNC_END:
                if (!funcReturn) {
                    if (strcmp(actual->res.s, "main") == 0) {
                        fprintf(out, "    mov $0, %%rax\n"); 
                    }
                    fprintf(out, "    leave\n");
                    fprintf(out, "    ret\n"); 
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
                break;
            case INSTR_PARAM: {
                const char* reg = regArg(paramIndex);
                if (reg) {
                    moverARegistro(reg, actual->arg1.s, out);
                } else {
                    cargarEnRAX(actual->arg1.s, out);
                    fprintf(out, "    push %%rax\n");
                    paramStackCount++;
                }
                paramIndex++;
                break;
            }
            case INSTR_CALL:
                fprintf(out, "    call %s\n", actual->arg1.s);
                if (paramStackCount > 0) {
                    fprintf(out, "    add $%d, %%rsp\n", paramStackCount * 8); 
                }
                paramIndex = 0;
                paramStackCount = 0;
                if (actual->res.s) {
                    guardarDesdeRAX(actual->res.s, out);
                }
                break;
            case INSTR_RETURN:
                if (actual->arg1.s) {
                    cargarEnRAX(actual->arg1.s, out);
                }
                fprintf(out, "    leave\n");
                fprintf(out, "    ret\n"); 
                funcReturn = 1;
                break;
            case INSTR_LABEL:
                fprintf(out, "%s:\n", actual->label.s); 
                funcReturn = 0;
                break;
            case INSTR_GOTO:
                fprintf(out, "    jmp %s\n", actual->label.s);
                break;
            case INSTR_IF:
                if (actual->arg1.s) {
                    if (esConstante(actual->arg1.s)) {
                        fprintf(out, "    mov $%s, %%rax\n", actual->arg1.s);
                        fprintf(out, "    cmp $0, %%rax\n");
                    } else {
                        
                        fprintf(out, "    cmp $0, %s\n", attAddr(actual->arg1.s));
                    }
                } else { 
                    fprintf(out, "    cmp $0, %%rax\n");
                }
                fprintf(out, "    je %s\n", actual->label.s); 
                break;
            case INSTR_WHILE:
                 if (actual->arg1.s) {
                    if (esConstante(actual->arg1.s)) {
                        fprintf(out, "    mov $%s, %%rax\n", actual->arg1.s);
                        fprintf(out, "    cmp $0, %%rax\n");
                    } else {
                        fprintf(out, "    cmp $0, %s\n", attAddr(actual->arg1.s));
                    }
                } else {
                    fprintf(out, "    cmp $0, %%rax\n");
                }
                fprintf(out, "    je %s\n", actual->label.s); 
                break;
            case INSTR_VAR_LOCAL:;
            break;
            default:
                fprintf(out, "    # Instrucción no implementada (%d)\n", actual->tipo);
                break;
        }

        actual = actual->next;
    }
}

// Devuelve el registro si el identificador es uno de los parámetros de la función
static const char* registroParametro(const char* nombre) {
    Simbolo* s = buscarSimbolo((char*)nombre);
    if (!s || !s->v) return NULL;

    if (s->v->esParametro && s->v->esParametro && s->v->offset >= 0 && s->v->offset < 6) {
        return regArg(s->v->offset);
    }

    return NULL;
}

// Devuelve el registro que le corresponde al indice del parámetro en una llamada [cite: 1222]
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
        fprintf(out, "    mov $%s, %%%s\n", src, reg); 
    } else {
        fprintf(out, "    movq %s, %%%s\n", attAddr(src), reg); 
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

// Chequea si la cadena representa una dirección de memoria (formato Intel [..])
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
        fprintf(out, "    mov %%%s, %%rax\n", op); 
        return;
    }

    const char* reg = registroParametro(op);
    if (reg) {
        fprintf(out, "    mov %%%s, %%rax\n", reg); 
        return;
    }

    if (esConstante(op)) {
        fprintf(out, "    mov $%s, %%rax\n", op);
    } else {
        fprintf(out, "    movq %s, %%rax\n", attAddr(op)); 
    }
}


// Carga el valor de una expresión en el registro rbx
static void cargarEnRBX(const char* op, FILE* out) {
    if (!op) return;

    if (esRegistro(op)) {
        fprintf(out, "    mov %%%s, %%rbx\n", op);
        return;
    }

    const char* reg = registroParametro(op);
    if (reg) {
        fprintf(out, "    mov %%%s, %%rbx\n", reg);
        return;
    }

    if (esConstante(op)) {
        fprintf(out, "    mov $%s, %%rbx\n", op);
    } else {
        fprintf(out, "    movq %s, %%rbx\n", attAddr(op));
    }
}


// Guarda lo que contiene rax en la dirección de destino
static void guardarDesdeRAX(const char* dst, FILE* out) {
    if (!dst) { fprintf(out, "    # guardarDesdeRAX(dst=NULL)\n"); return; }
    fprintf(out, "    movq %%rax, %s\n", attAddr(dst)); 
}

// Funcion auxiliar para traducir la operación de asignación de código intermedio a código objeto
static void asignarTraduccion(FILE* out, const char* dst, const char* src) {
    if (!dst || !src) {
        fprintf(out, "    # asignar(dst/src NULL)\n");
        return;
    }
    if (esConstante(src)) {
        fprintf(out, "    movq $%s, %s\n", src, attAddr(dst)); 
    }else { 
        cargarEnRAX(src, out);
        guardarDesdeRAX(dst, out);
    }
}