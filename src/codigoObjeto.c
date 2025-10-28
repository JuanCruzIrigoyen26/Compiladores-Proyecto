#include <stdio.h>
#include <stdlib.h>
#include "codigoObjeto.h"

// Función auxiliar para traducir operaciones aritméticas
static void traducirOperacion(Instr* instr, FILE* out) {
    if (!instr) return;

    switch (instr->tipo) {
        case INSTR_ADD:
            fprintf(out, "    MOV rax, [%s]\n", instr->arg1.s);
            fprintf(out, "    ADD rax, [%s]\n", instr->arg2.s);
            fprintf(out, "    MOV [%s], rax\n", instr->res.s);
            break;
        case INSTR_SUB:
            fprintf(out, "    MOV rax, [%s]\n", instr->arg1.s);
            fprintf(out, "    SUB rax, [%s]\n", instr->arg2.s);
            fprintf(out, "    MOV [%s], rax\n", instr->res.s);
            break;
        case INSTR_MUL:
            fprintf(out, "    MOV rax, [%s]\n", instr->arg1.s);
            fprintf(out, "    IMUL rax, [%s]\n", instr->arg2.s);
            fprintf(out, "    MOV [%s], rax\n", instr->res.s);
            break;
        case INSTR_DIV:
            fprintf(out, "    MOV rax, [%s]\n", instr->arg1.s);
            fprintf(out, "    XOR rdx, rdx\n");
            fprintf(out, "    IDIV [%s]\n", instr->arg2.s);
            fprintf(out, "    MOV [%s], rax\n", instr->res.s);
            break;
        case INSTR_ASSIGN:
            fprintf(out, "    MOV [%s], %s\n", instr->res.s, instr->arg1.s);
            break;
        case INSTR_MOD:
            // TODO: Implementar operación módulo
            break;
        case INSTR_LT:
        case INSTR_GT:
        case INSTR_EQ:
        case INSTR_AND:
        case INSTR_OR:
        case INSTR_NOT:
            // TODO: Implementar comparaciones y operadores lógicos
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

    fprintf(out, "section .text\n");
    fprintf(out, "global main\n\n");

    Instr* actual = generador->head;
    while (actual) {
        switch (actual->tipo) {
            case INSTR_FUNC_BEGIN:
                fprintf(out, "\n; Inicio de función %s\n%s:\n", actual->res.s, actual->res.s);
                fprintf(out, "    push rbp\n");
                fprintf(out, "    mov rbp, rsp\n");
                break;
            case INSTR_FUNC_END:
                fprintf(out, "    mov rsp, rbp\n");
                fprintf(out, "    pop rbp\n");
                fprintf(out, "    ret\n");
                fprintf(out, "; Fin de función %s\n", actual->res.s);
                break;
            case INSTR_VAR_GLOBAL:
                fprintf(out, "section .data\n%s dq 0\n", actual->res.s);
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
            case INSTR_PARAM:
                fprintf(out, "    ; PARAM %s (pendiente manejo de stack)\n", actual->arg1.s);
                break;
            case INSTR_CALL:
                fprintf(out, "    CALL %s\n", actual->arg1.s);
                if (actual->res.s)
                    fprintf(out, "    MOV [%s], rax\n", actual->res.s);
                break;
            case INSTR_RETURN:
                if (actual->arg1.s)
                    fprintf(out, "    MOV rax, %s\n", actual->arg1.s);
                fprintf(out, "    RET\n");
                break;
            case INSTR_RETVAL:
                // TODO: Implementar manejo de retorno de valor
                break;
            case INSTR_LABEL:
                fprintf(out, "%s:\n", actual->label.s);
                break;
            case INSTR_GOTO:
                fprintf(out, "    JMP %s\n", actual->label.s);
                break;
            case INSTR_IF:
                fprintf(out, "    CMP %s, 0\n", actual->arg1.s);
                fprintf(out, "    JNE %s\n", actual->label.s);
                break;
            case INSTR_WHILE:
                fprintf(out, "    ; WHILE %s -> %s\n", actual->arg1.s, actual->res.s);
                fprintf(out, "    CMP %s, 0\n", actual->arg1.s);
                fprintf(out, "    JE %s\n", actual->res.s);
                break;
            default:
                fprintf(out, "    ; Instrucción no implementada (%d)\n", actual->tipo);
                break;
        }

        actual = actual->next;
    }

    fprintf(out, "\n; Fin del código objeto\n");
}
