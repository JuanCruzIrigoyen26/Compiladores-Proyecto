# Compiladores-Proyecto

Integrantes: Irigoyen Juan Cruz - Marysol Gutierrez

Esta es la rama de prueba para el árbol de sintaxis abstracta, la tabla de símbolos y el analizador semántico.

## Compilación y Ejecución

Para compilar y ejecutar los archivos escriba los siguientes comandos en terminal:

```

bison -d bison.y
flex lexico.l
gcc -o parser bison.tab.c lex.yy.c ast.c ts.c analizador_semantico.c main.c -lfl
./parser tests/nombreTest.txt

```

En la carpeta tests se encuentran diferentes ejemplos para probar el parser. Por ejemplo: tests/test1.txt
