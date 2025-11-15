#ifndef AST_H
#define AST_H

// Tipos de nodos en el AST
typedef enum {
    AST_PROG,         // Programa completo
    AST_DECL_VAR,     // Declaración de variable
    AST_DECL_FUNC,    // Declaración/definición de función
    AST_PARAM,        // Parámetro individual
    AST_PARAMS,       // Lista de parámetros
    AST_BLOQUE,       // Bloque de sentencias
    AST_DECLS,        // Lista de declaraciones
    AST_STMTS,        // Lista de sentencias
    AST_SEQ_EXPR,    // Lista de expresiones
    AST_ASIGNACION,   // Asignación
    AST_IF,           // If / If-Else
    AST_WHILE,        // While
    AST_RETURN,       // Return
    AST_LLAMADA,      // Llamada a función
    AST_OP,           // Operación (suma, resta, etc.)
    AST_ID,           // Identificador
    AST_INT,          // Literal entero
    AST_BOOL,          // Literal booleano
    AST_VOID
} AstTipo;


// Tipo de dato: entero o booleano
typedef enum {
    INT,
    BOOL,
    VOID
} tipoDef;

// Operadores soportados
typedef enum {
    OP_SUMA, 
    OP_RESTA, 
    OP_MULT, 
    OP_DIV, 
    OP_MOD,
    OP_ASIG,      
    OP_IGUAL,         
    OP_MAYOR, 
    OP_MENOR, 
    OP_AND, 
    OP_OR, 
    OP_NOT
} TipoOp;

// Valor asociado a cada nodo del AST
typedef struct {
    tipoDef tipoDef; // Tipo del valor (INT o BOOL)
    long   i;        // Valor si es entero
    int    b;        // Valor si es booleano
    char  *s;        // Nombre si es identificador
    TipoOp op;       // Operador si es nodo de operación
    int flag;        // Marca si la variable fue inicializada
    int esFuncion;   // Marca si es funcion o variable
    int esParametro; // Marca si es parametro
    int linea;       // Línea donde aparece el token
    int offset;      // Variable o temporal local en la pila
    int esExtern;    // 1 si la función es extern, 0 si no
    int esBloqueDeFuncion; 
} AstValor;

// Nodo del AST
typedef struct Nodo {
    AstTipo tipo;    // Tipo de nodo (AST_INT, AST_OP, etc.)
    AstValor *v;     // Valor asociado al nodo
    struct Nodo *hi; // Hijo izquierdo (para binarios o secuencias)
    struct Nodo *hd; // Hijo derecho
    struct Nodo *extra; 
} Nodo;


// Funciones auxiliares para construir el AST
Nodo* nodo_hoja(AstTipo t, AstValor v); // Crea un nodo hoja
Nodo* nodo_binario(AstTipo t, AstValor v, Nodo* hi, Nodo* hd); // Crea un nodo binario
Nodo* nodo_ternario(AstTipo t, AstValor v, Nodo* hi, Nodo* hd, Nodo* extra); // Crea un nodo ternario
void imprimir_ast(FILE *out, Nodo* nodo, int nivel); // Muestra el AST 
Nodo* nodo_id_simbolo(AstValor *v);

#endif