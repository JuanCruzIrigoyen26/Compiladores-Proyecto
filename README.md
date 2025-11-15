# Compiladores-Proyecto

Integrantes: Irigoyen Juan Cruz - Marysol Gutierrez

Proyecto centrado en el desarrollo de un compilador para el lenguaje de programación TDS25.
En esta rama se encuentra la última versión del proyecto actual. 

## Ramas de Prueba

Se han creado las siguientes ramas para probar las diferentes funcionalidades que se implementen durante el transcurso de desarrollo del proyecto:

- Main (Última versión del proyecto)
- scanner-parser (Rama de prueba del analizador léxico y sintáctico)
- ast-ts (Rama de Prueba del árbol de sintaxis abstracta, tabla de símbolos y analizador semantico)
- codigo_intermedio (Rama de prueba del codigo intermedio)
- codigo_objeto (Rama de prueba del codigo objeto)
- interfaz_cmd (Rama de prueba de la interfaz de comandos)

## Comandos de ejecución

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
