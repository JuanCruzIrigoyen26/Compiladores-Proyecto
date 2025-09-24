# Compiladores-Proyecto

Integrantes: Irigoyen Juan Cruz - Marysol Gutierrez

Esta es la rama de prueba para el árbol de sintaxis abstracta y la tabla de símbolos.

## Compilación y Ejecución

Para compilar y ejecutar los archivos escriba los siguientes comandos en terminal:

```

bison -d bison.y
flex lexico.l
gcc -o parser bison.tab.c lex.yy.c ast.c ts.c main.c -lfl
./parser tests/nombreTest.txt

```

En la carpeta tests se encuentran diferentes ejemplos para probar el parser. Por ejemplo: tests/test1.txt
