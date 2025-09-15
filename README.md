# Compiladores-Proyecto

Integrantes: Irigoyen Juan Cruz - Marysol Gutierrez

En esta rama se encuentra unicamente el scanner y parser para su prueba.

## Compilación y Ejecución

Para compilar y ejecutar los archivos escriba los siguientes comandos en terminal:

```

bison -d bison.y
flex lexico.l
gcc -o parser bison.tab.c lex.yy.c -lfl
./parser < tests/nombreTest.txt

```

En la carpeta tests se encuentran diferentes ejemplos para probar el parser. Por ejemplo: tests/test1.txt
