#include "hilbert.h"
#include "RTree.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int n_hilbert = pow(2, 19);

void rot(unsigned long long int n, unsigned long long int *x, unsigned long long int *y, unsigned long long int rx, unsigned long long int ry) {
    unsigned long long int t;
    if (ry == 0) {
        if (rx == 1) {
            *x = n-1 - *x;
            *y = n-1 - *y;
        }
        t  = *x;
        *x = *y;
        *y = t;
    }
}

//convierte (x,y) a d
unsigned long long int xy2d (unsigned long long int x, unsigned long long int y) {
    unsigned long long int n = (unsigned long long int)n_hilbert;
    unsigned long long int rx, ry, s;
    unsigned long long int d = 0;
    for (s = n/2; s > 0; s /= 2) {
        rx = (x & s) > 0;
        ry = (y & s) > 0;
        /* printf("x: %lld, y: %lld, rx: %lld, ry: %lld, (3*rx) ^ ry: %lld, s: %lld, d: %lld\n", x, y, rx, ry, (3 * rx) ^ ry, s, d); */
        d += s * s * ((3 * rx) ^ ry);
        rot(s, &x, &y, rx, ry);
    }
    return d;
}

int compareCentersByHilbert(const void *r1, const void *r2) {
    Node *a = *(Node **) r1;
    Node *b = *(Node **) r2;

    Point c1 = getRectCenter(a->MBR);
    Point c2 = getRectCenter(b->MBR);

    /* printf("==============================================================================\n"); */
    /* printf("C1: (%.2f, %.2f)\n", c1.x, c1.y); */
    unsigned long long int d1 = xy2d((unsigned long long int)c1.x, (unsigned long long int)c1.y);
    /* printf("C2: (%.2f, %.2f)\n", c2.x, c2.y); */
    unsigned long long int d2 = xy2d((unsigned long long int)c2.x, (unsigned long long int)c2.y);
    /* printf("==============================================================================\n"); */


    if (d1 < d2) return -1;
    else if (d1 > d2) return 1;
    return 0;
}

void sortNodesByHilbert(Node **nodes, int n) {
    qsort(nodes, n, sizeof(Node*), compareCentersByHilbert);
};

RTree generateTreeHilbert(Node **nodes, int n, int M, int debug) {
    /* Calcular grado para curva de Hilbert */
    if (debug) printf("Grado curva de Hilbert: %d\n", n_hilbert);

    /* Calcular cantidad de árboles necesarios */
    int nTrees = (int)ceil(n / (float)M);

    if (debug) printf("Cantidad de nodos: %d\n", n);
    if (debug) printf("Arboles a construir: %d\n", nTrees);

    /* Generar arboles */
    while (nTrees > 1) {
        if (debug) printf("Ordenando nodos...\n");
        sortNodesByHilbert(nodes, n);

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