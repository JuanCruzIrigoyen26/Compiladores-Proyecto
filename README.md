# Compiladores-Proyecto

Integrantes: Irigoyen Juan Cruz - Marysol Gutierrez

En esta rama se implementa la **generación de código objeto** del compilador.

## Descripción

En esta etapa se toma como entrada el **código intermedio** generado previamente y se produce un **código objeto en pseudo-assembly x86-64**, mostrado por pantalla.  
El objetivo es traducir correctamente las instrucciones de tres direcciones a instrucciones en bajo nivel (sin optimizaciones).

## Compilación y Ejecución

Para compilar y ejecutar los archivos escriba los siguientes comandos en terminal:

```

bison -d bison.y
flex lexico.l
gcc -o parser bison.tab.c lex.yy.c ast.c ts.c analizador_semantico.c codigoIntermedio.c codigoObjeto.c main.c -lfl
./parser tests/nombreTest.txt

```

En la carpeta tests se encuentran diferentes ejemplos para probar el parser. Por ejemplo: tests/test1.txt
