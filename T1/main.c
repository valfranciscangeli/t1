#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <math.h>
#include "nearest_x.h"
#include "hilbert.h"
#include "STR.h"
#define PAGESIZE 4096

void initRectangles(Rect *R, int n, int max_coords, int max_largo_R, int debug) {
    if (debug) printf("Inicializando rectángulos\n");
    for (int i=0; i<n; i++) {
        int x0 = rand() % max_coords, y0 = rand() % max_coords;
        int x1 = x0 + (rand() % max_largo_R), y1 = y0 + (rand() % max_largo_R);

        if (debug) printf("Coordenadas: (%d, %d) - (%d, %d)\n", x0, y0, x1, y1);
        if (debug) printf("Puntero a rectángulo %d: %p\n", i+1, &R[i]);

        initRect(&R[i], (float)x0, (float)y0, (float)x1, (float)y1);
    }
};

int main() {
    int debug = 0;
    /* Se inicializa srand */
    unsigned int seed = 31415;
    srand(seed);

    /* Limites distribuciones */
    int max_coords = 500000-100; // ya que al calcular el random no considera la diferencia
    int max_largo_R = 100;
    int max_largo_Q = 100000;

    /* Inicializar conjunto R y valor M */
    int nArr[16];
    int nq = 100;
    int M = (PAGESIZE-2)/ (28);

    nArr[0] = 1024;
    for (int i=1; i<16; i++) {
        nArr[i] = nArr[i-1]*2;
    }

    /* Definir variables para cálculos de tiempos de ejecución */
    clock_t start, end;
    double tempo;
    
    /* Inicializar nombres para directorios de archivos */
    char *rootDirs[] = {
        "disco/nearestX",
        "disco/hilbert",
        "disco/STR"
    };

    /* Reservar memoria para Q */
    if (debug) printf("Reservando memoria para rectángulos de consulta\n");
    Rect *Q = (Rect *) malloc(nq * sizeof(Rect));

    /* Inicializar array Q */
    initRectangles(Q, nq, max_coords, max_largo_Q, debug);
    
    for (int a=0; a<(sizeof(nArr) / sizeof(int)); a++) {
        /* Array para comparar resultados de búsquedas */
        /* Node **expectedNodes; */

        /* Definir n */
        int n = nArr[a];
        printf("\n---- n = %d ** %d ----\n", 2, a + 10);

        /* Reservar memoria para R */
        if (debug) printf("Reservando memoria para rectángulos del árbol\n");
        Rect *R = (Rect *) malloc(n * sizeof(Rect));

        /* Inicializar array R */
        initRectangles(R, n, max_coords, max_largo_R, debug);

        /* Iterar para utilizar los 3 métodos de generación */
        for (int t=0; t<3; t++) {
            printf("------------------ Usando disco en %s ------------------\n", rootDirs[t]);
            /* Reservar memoria para Nodos */
            if (debug) printf("Reservando memoria para nodos\n");
            Node **nodes = (Node **) malloc(n * sizeof(Node *));

            /* Incorporar rectángulos en Nodos */
            if (debug) printf("Incorporando rectángulos en nodos\n");
            for (int i=0; i<n; i++) {
                nodes[i] = malloc(sizeof(Node));
                initNode(nodes[i]);
                nodes[i]->MBR = R[i];
                if (debug) {
                    printf("Nodo %d\n", i+1);
                    printNode(*(nodes[i]));
                    printf("\n");
                }
            };

            RTree finalTree;
            
            start = clock();
            if (t==0) {
                finalTree = generateTreeNearestX(nodes, n, M, debug);
            }
            else if (t==1) {
                finalTree = generateTreeHilbert(nodes, n, M, debug);
            }
            else if (t==2) {
                finalTree = generateTreeSTR(nodes, n, M, debug);
            }
            end = clock();

            tempo = (double)(end - start) / CLOCKS_PER_SEC;
            if (debug) printf("Duración armado árbol [s]: \t\t%.2f seg.\n", tempo);

            /* Entero para guardar cantidad de nodos escritos */
            int nw = 0;

            if (debug) {
                printf("Arbol generado :D\n");
                printRTree(finalTree);
                printf("\n");
                printf("Altura del arbol generado: %d\n", treeHeight(&finalTree));
                printf("Árbol válido: %s\n", isValidTree(&finalTree) ? "True" : "False");
            }

            /* Escribiendo arbol en disco */
            if (debug) printf("Escribiendo árbol en disco.\n");
            
            char diskFilename[50];
            strcpy(diskFilename, rootDirs[t]);
            strcat(diskFilename, "/disk.bin");
            if (debug) printf("Nombre archivo de disco: %s\n", diskFilename);

            FILE *wrt_ptr;
            int offset = 0;

            /* Limpiar la data previa del disco */
            wrt_ptr = fopen(diskFilename, "wb");
            fclose(wrt_ptr);
            wrt_ptr = fopen(diskFilename, "rb+");
            
            start = clock();
            writeTreeToBin(&finalTree, wrt_ptr, &offset, &nw, debug);
            end = clock();
            if (debug) printf("Árbol escrito. %d nodos escritos.\n", nw);

            /* Liberando memoria del arbol */
            freeRTree(&finalTree);
            
            /* Liberando memoria de nodos  */
            if (debug) printf("Liberando memoria del aray de nodos\n");
            free(nodes);
            
            tempo = (double)(end - start) / CLOCKS_PER_SEC;
            if (debug) printf("Duración escritura en disco [s]: \t%.2f seg.\n", tempo);

            fclose(wrt_ptr);

            /* Determinando referencia raiz */
            char treeRef[9] = "00000000";

            if (debug) printf("Abriendo disco para lectura\n");
            FILE *rd_ptr;
            rd_ptr = fopen(diskFilename, "rb");

            /* Creando archivo de resultados */
            char resultsFilename[50];
            sprintf(resultsFilename, "%s/resultados_%d.csv", rootDirs[t], a+10);
            if (debug) printf("Nombre archivo de disco: %s\n", resultsFilename);
            FILE *results_ptr;
            results_ptr = fopen(resultsFilename, "w");

            /* Escribiendo encabezado */
            char encabezado[] = "i,t,IO\n";
            fwrite(encabezado, 1, strlen(encabezado), results_ptr);

            /* Se realizan las búsquedas */
            double totalTime = 0;
            int totalIO = 0;
            for (int i=0; i<nq; i++) {
                /* Se borra caché */
                system("sudo sh -c \"/usr/bin/echo 3 > /proc/sys/vm/drop_caches\"");

                Rect q = Q[i];

                if (debug) {
                    printf("------------------");
                    printf(" Directorio raíz: %s ", rootDirs[t]);
                    printf("------------------\n");
                    printf("Buscando rectángulo ");
                    printRect(q);
                    printf(" en árbol final con referencia %s\n", treeRef);
                }
                
                int blocksIO = 0, lenRes = 0;
                
                Rect **resultRects = (Rect **) malloc(n * sizeof(Rect *));
                if (debug) printf("ptr inicial: %p\n", resultRects);

                start = clock();
                searchRect(rd_ptr, 0, q, &blocksIO, &lenRes, resultRects, debug);
                end = clock();

                tempo = (double)(end - start) / CLOCKS_PER_SEC;

                totalTime += tempo;
                totalIO += blocksIO;

                /* Escribiendo en archivo de resultados */
                char resultRow[50];
                sprintf(resultRow, "%d,%.6f,%d\n", i, tempo, blocksIO);
                fwrite(resultRow, 1, strlen(resultRow), results_ptr);

                if (debug) {
                    printf("Cantidad nodos árbol: \t\t\t\t%d\n", nw);    
                    printf("Cantidad de rectangulos que intersectan: \t%d\n", lenRes);
                    printf("Operaciones I/O: \t\t\t\t%d\n", blocksIO);
                    printf("Duración búsqueda [s]: \t\t\t\t%.2f\n", tempo);
                    
                    for (int j=0; i<lenRes; j++) {
                        printf("Rectángulo resultante %d:\n", j+1);
                        printRect(*(resultRects[j]));
                        printf("\n");
                    }
                }

                if (debug) printf("Liberando memoria de resultados\n");
                for (int j=0; j<lenRes; j++) {
                    free(resultRects[j]);
                }
                free(resultRects);
            }

            printf("Tiempo promedio de búsqueda: \t\t%.6f seg\n", totalTime / nq);
            printf("Cantidad promedio de operaciones IO: \t%.3f\n", totalIO / (float)nq);

            /* Cerrar archivo de resultados */
            fclose(results_ptr);

            if (debug) printf("Cerrando archivo de disco\n");
            fclose(rd_ptr);

        }
        if (debug) printf("Liberando memoria del conjunto R\n");
        free(R);
    }
    if (debug) printf("Liberando memoria del conjunto Q\n");
    free(Q);

    return 0;
}

/*
Posibilidades de mejora:
    Representar rectángulo con su punto menor y sus largos
    Centro sería la suma entre el punto menor y la mitad de los largos respectivos y habría 
    reducción del tamaño en disco
*/