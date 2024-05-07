#include "STR.h"
#include "nearest_x.h"
#include "RTree.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int compareCentersByY(const void *r1, const void *r2) {
    Node *a = *(Node **) r1;
    Node *b = *(Node **) r2;

    Point c1 = getRectCenter(a->MBR);
    Point c2 = getRectCenter(b->MBR);

    if (c1.y < c2.y) return -1;
    else if (c1.y > c2.y) return 1;
    return 0;
}

void sortNodesByYCenter(Node **nodes, int n) {
    qsort(nodes, n, sizeof(Node*), compareCentersByY);
};

RTree generateTreeSTR(Node **nodes, int n, int M, int debug) {
    /* Calcular cantidad de árboles necesarios */
    int nTrees = (int)ceil(n / (float)M);
    int S = (int)ceil(sqrt(nTrees));

    if (debug) printf("Cantidad de nodos: %d\n", n);
    if (debug) printf("Arboles a construir: %d\n", nTrees);
    if (debug) printf("S: %d\n", S);

    /* Generar arboles */
    while (nTrees > 1) {
        /* Ordenar nodos según coordenada X del centro */
        if (debug) printf("Ordenando nodos según X...\n");
        sortNodesByXCenter(nodes, n);

        /* Reservar la memoria para los árboles */
        RTree *rTrees = (RTree *) malloc(nTrees * sizeof(RTree));
        
        /* Inicializar variables */
        int count = 0;
        RTree *actualTree;

        /* Generar grupos verticales con S*M nodos */
        for (int i=0; i<n; i=i+S*M) {
            /* Copiar slice de los nodos correspondientes al grupo vertical */
            int groupSize = S*M < (n-i) ? S*M : (n-i);
            if (debug) printf("Generando grupo vertical %d de tamaño %d\n", i/(S*M)+1, groupSize);
            Node **vGroup = malloc(groupSize * sizeof(Node *));

            for (int j=0; j<groupSize; j++) {
                vGroup[j] = nodes[i+j];
            }

            /* Ordenar el grupo de nodos según la coordenada Y de su centro */
            sortNodesByYCenter(vGroup, groupSize);

            /* Poblar árboles con nodos */
            for (int j=0; j<groupSize; j++) {
                /* Si j es múltiplo de M, se inicializa un nuevo árbol */
                if (j%M == 0) {
                    /* Inicializar árbol */
                    actualTree = &(rTrees[count]);
                    count++;
                    initRTree(actualTree, M);
                }

                /* Agregar nodo al árbol */
                if (debug) printf("Agregando nodo %d al arbol %d\n", i+j, count);
                addNode(actualTree, vGroup[j]);
            }

            /* Liberar memoria */
            free(vGroup);
        }

        /* Si se hizo más de un árbol, se inserta cada árbol como hijo de un nodo */
        if (debug) printf("Insertando arboles en nodos\n");
        for (int i=0; i<nTrees; i++) {
            /* Inicializar nodo y capturar RTree */
            if (debug) printf("Insertando arbol %d\n", i+1);
            nodes[i] = (Node *) malloc(sizeof(Node));
            initNode(nodes[i]);
            RTree *rt = &(rTrees[i]);
            
            /* Se asigna el RTree como hijo del nodo y se calcula su MBR */
            nodes[i]->child = rt;
            setNodeMBR(nodes[i]);
        };

        /* Se re-calculan las variables n y nTrees */
        n = nTrees;
        nTrees = (int)ceil(n / (float)M);
        S = (int)ceil(sqrt(nTrees));
        
        if (debug) printf("Cantidad de nodos: %d\n", n);
        if (debug) printf("Arboles a construir: %d\n", nTrees);
        if (debug) printf("S: %d\n", S);
    };

    /* Se genera el árbol faltante con los nodos restantes */
    RTree finalRT;
    if (debug) printf("\nInicializando arbol final\n");
    initRTree(&finalRT, M);
    for (int i=0; i<n; i++) {
        if (debug) printf("Agregando nodo %d al arbol final\n", i+1);
        if (debug) printNode(*(nodes[i]));
        addNode(&finalRT, nodes[i]);
    };

    return finalRT;
}