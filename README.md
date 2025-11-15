# Compiladores-Proyecto

Integrantes: Irigoyen Juan Cruz - Marysol Gutierrez

En esta rama se implementa la **generación de código objeto** del compilador.

## Descripción

En esta etapa se toma como entrada el **código intermedio** generado previamente y se produce un **código objeto en pseudo-assembly x86-64**, mostrado por pantalla.  
El objetivo es traducir correctamente las instrucciones de tres direcciones a instrucciones en bajo nivel (sin optimizaciones).

## Compilación y Ejecución

Para compilar los archivos escriba los siguientes comandos en terminal:

```

bison -d bison.y
flex lexico.l
gcc -o c-tds main.c bison.tab.c lex.yy.c ast.c ts.c analizador_semantico.c codigoIntermedio.c codigoObjeto.c -lfl

```

Para generar el archivo ejecutable escriba el siguiente comando en terminal:

```

 ./c-tds nombreArchivo.ctds

```

Para generar el archivo de salida de la etapa deseada (scan, parse, sem, codinter, assembly) escriba el siguiente comando en terminal:

```

 ./c-tds -target (etapa) nombreArchivo.ctds

```

Para renombrar el archivo ejecutable escriba el siguiente comando en terminal:

```

-o nombreArchivo

```

## Carpetas de testing

Dentro de las carpetas de tests, se encuentra casos de programas del lenguaje TDS25 que se ejecutan correctamente, casos de errores semánticos y casos de errores sintácticos.
