#include "nearest_x.h"
#include "RTree.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int compareCentersByX(const void *r1, const void *r2) {
    Node *a = *(Node **) r1;
    Node *b = *(Node **) r2;

    Point c1 = getRectCenter(a->MBR);
    Point c2 = getRectCenter(b->MBR);

    if (c1.x < c2.x) return -1;
    else if (c1.x > c2.x) return 1;
    return 0;
}

void sortNodesByXCenter(Node **nodes, int n) {
    qsort(nodes, n, sizeof(Node*), compareCentersByX);
};

RTree generateTreeNearestX(Node **nodes, int n, int M, int debug) {
/* Calcular cantidad de árboles necesarios */
    int nTrees = (int)ceil(n / (float)M);
    if (debug) printf("Cantidad de nodos: %d\n", n);
    if (debug) printf("Arboles a construir: %d\n", nTrees);

    /* Generar arboles */
    while (nTrees > 1) {
        if (debug) printf("Ordenando nodos...\n");
        sortNodesByXCenter(nodes, n);

        /* Reservar la memoria para los árboles */
        RTree *rTrees = (RTree *) malloc(nTrees * sizeof(RTree));

        /* Poblar árboles con nodos */
        for (int i=0; i<nTrees; i++) {
            /* Inicializar arbol */
            RTree *rt = &(rTrees[i]);
            initRTree(rt, M);

            /* Agregar nodos al arbol (desde el i*M hasta el menor entre (i+1) * M y n) */
            int j;
            int maxj = (i+1) * M < n ? (i+1) * M : n;
            for (j=i*M; j < maxj; j++) {
                if (debug) printf("Agregando nodo %d al arbol %d\n", j+1, i+1);
                addNode(rt, nodes[j]);
            }

            if (debug) printRTree(*rt);
        }
        if (debug) printf("\n");
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
        
        if (debug) printf("Cantidad de nodos: %d\n", n);
        if (debug) printf("Arboles a construir: %d\n", nTrees);
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