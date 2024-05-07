# Ejecución del código
1. Descomprimir fichero
2. Ejecutar `sudo bash` para permitir la ejecución y borrado de caché de las páginas de disco
3. (Compilar) Ejecutar: `gcc -Wall -g -o main main.c nearest_x.c hilbert.c STR.c RTree.c -lm`
4. (Ejecución) Ejecutar: `./main`

Asegurarse de la existencia de los directorios `disco/nearestX`, `disco/hilbert` y `disco/STR` en el directorio de ejecución del archivo main.

Se tiene la posibilidad de ajustar diferentes parámetros de ejecución modificando sus valores en las primeras líneas de la funcion `main` del archivo `main.c`:
- `int debug`: (default: 0) Modo de debug del sistema. Imprimirá numerosos mensajes de debug de la creación de los rectángulos, coordenadas, escritura y lectura de disco, etc.
- `unsigned int seed`: (default: 31415) Semilla del generador de números aleatorios. Se utiliza de manera estática para tener consistencia en los experimentos y resultados.
- `int max_coords`: (default: 499900) Valor máximo que toman las coordenadas del punto inferior izquierdo de cualquier rectángulo.
- `int max_largo_R`: (default: 100) Tamaño máximo del lado de los rectángulos del conjunto $R$.
- `int max_largo_Q`: (default: 100000) Tamaño máximo del lado de los rectángulos del conjunto $Q$.
- `int nArr[]`: (default: $\{2^{10}, (...), 2^{25}\}$ Array que agrupa los diferentes $n$ de rectángulos con los que se testeará el sistema.
- `int nq`: (default 100) Número de consultas que se realizarán con cada método de construcción, para cada $n$ en `nArr`
- `int M`: (default: 146) Cantidad máxima de rectángulos que contendrá cada nodo del árbol.

* Si se va a modificar el array `nArr`, hay que tener consistencia con el contenido impreso en el primer `printf()` sin debug, ya que está pensado en imprimir partiendo desde $2^{10}$.
