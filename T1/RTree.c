#include "RTree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/* RTree */
void initRTree(RTree *rt, int size) {
    rt->n = 0;
    rt->nodes = malloc(size * sizeof(Node *));
};

void addNode(RTree *rt, Node *node) {
    rt->nodes[rt->n] = node;
    rt->n++;
};

Rect calcMBR(RTree rt) {
    int i, n = rt.n;
    Rect MBR = rt.nodes[0]->MBR;

    for (i=1;i<n;i++) {
        Node *node = rt.nodes[i];
        MBR.p1 = getMinPoint(MBR.p1, node->MBR.p1);
        MBR.p2 = getMaxPoint(MBR.p2, node->MBR.p2);
    };

    return MBR;
};

int treeHeight(RTree *prt) {
    int final_h = 1, max_child_h = 0, i;
    RTree rt = *prt;

    for (i=0; i < rt.n; i++) {
        if (!isLeaf(*(rt.nodes[i]))) {
            int node_height = treeHeight(rt.nodes[i]->child);
            max_child_h = max_child_h > node_height ? max_child_h : node_height;
        }
    }
    
    return final_h + max_child_h;
}

int isValidTree(RTree *prt) {
    /* Solo se revisa que la altura de los hijos sea igual */
     int valid = 1, i;
     RTree rt = *prt;

    if (rt.n == 0) return valid;

    int expected_height = treeHeight(rt.nodes[0]->child);

    for (i=0; i<rt.n; i++) {
        valid = valid && treeHeight(rt.nodes[i]->child) == expected_height;
    }

    return valid;
}

void searchRect(FILE *rd_ptr, int offset, Rect r, int *blocksIO, int *count, Rect **resultRects, int debug) {
    /* Se lee la data del archivo de la referencia */
    int nr = 0;
    if (debug) printf("Leyendo data del disco en el offset %d\n", offset); 
    char **data = readBin(rd_ptr, offset, &nr, blocksIO, debug);

    for (int i=0; i<nr; i++) {
        if (debug) printf("Parseando data rectángulo %d: %s\n", i+1, data[i]);
        /* Se extraen los datos del nodo */
        char sx1[6], sy1[6], sx2[6], sy2[6];
        char refNode[10];
        sscanf(data[i], "%5s%5s%5s%5s%8s", sx1, sy1, sx2, sy2, refNode);

        float x1, y1, x2, y2;
        int offsetNode;
        x1 = (float)strtol(sx1, NULL, 16);
        y1 = (float)strtol(sy1, NULL, 16);
        x2 = (float)strtol(sx2, NULL, 16);
        y2 = (float)strtol(sy2, NULL, 16);
        offsetNode = (int)strtol(refNode, NULL, 16);
        
        if (debug) printf("Data extraída del buffer: (%.2f, %.2f) - (%.2f, %.2f) (%d)\n", x1, y1, x2, y2, offsetNode);

        /* Se genera un rectángulo para guardar la data del MBR del nodo*/
        Rect *nodeRect = (Rect *)malloc(sizeof(Rect));
        initRect(nodeRect, x1, y1, x2, y2);

        /* Se revisa si el MBR intersecta con el rectángulo de la búsqueda */
        if (areIntersecting(r, *nodeRect)) {
            /* Si el nodo es hoja (su referencia es 0), se guarda el nodo en el array de resultados */
            if (strcmp(refNode, "XXXXXXXX") == 0) {
                resultRects[*count] = nodeRect;
                (*count)++;
            }
            /* Si el nodo no es hoja, se continua la búsqueda en la referencia al hijo y se libera la memoria */
            else {
                free(nodeRect);
                searchRect(rd_ptr, offsetNode, r, blocksIO, count, resultRects, debug);
            }
        }
        /* Si no intersectan, se libera la data */
        else {
            free(nodeRect);
        }
    }

    /* Liberar memoria para data */
    for (int i=0; i<nr; i++) {
        free(data[i]);
    }
    free(data);

    return;
}

void writeTreeToBin(RTree *prt, FILE *wrt_ptr, int *offset, int *nw, int debug) {
    /* Cada nodo será escrito como una cadena de NNXXXXXYYYYYXXXXXYYYYYRRRRRRRR siendo:
        NN:    Cantidad de nodos escritos en el archivo,
        XXXXX: Cadena con padding de 0 para el entero X del punto 1 del MBR del nodo,
        YYYYY: Cadena con padding de 0 para el entero Y del punto 1 del MBR del nodo,
        XXXXX: Cadena con padding de 0 para el entero X del punto 2 del MBR del nodo,
        YYYYY: Cadena con padding de 0 para el entero Y del punto 2 del MBR del nodo,
        RRRRRRRR: Referencia al offset del nodo hijo.
        Todos los valores están en hexadecimal. */

    /* Se reserva memoria para el buffer de datos a escribir  */
    char buffer[8192];
    buffer[0] = '\0';

    /* Se escribe en el lengthBuffer la cantidad de nodos a agregar */
    char lengthBuffer[3];
    sprintf(lengthBuffer, "%02X", prt->n);
    strncat(buffer, lengthBuffer, 3);

    if (debug) printf("\nNueva escritura. Iniciando en offset %05d\n", *offset);

    /* Se escribe en el buffer la data de los nodos del árbol */
    for (int i=0; i<prt->n; i++) {
        char nodeBuffer[50];
        sprintf(nodeBuffer, "%05X%05X%05X%05X%s",
            (int)(prt->nodes[i]->MBR.p1.x),
            (int)(prt->nodes[i]->MBR.p1.y),
            (int)(prt->nodes[i]->MBR.p2.x),
            (int)(prt->nodes[i]->MBR.p2.y),
            "XXXXXXXX");

        strncat(buffer, nodeBuffer, 30);
    }

    /* Se escribe el buffer en el archivo y se libera el buffer */
    int writtenBytes = strlen(buffer);
    fseek(wrt_ptr, (*offset), SEEK_SET);
    fwrite(buffer, writtenBytes, 1, wrt_ptr);
    (*nw)++;

    if (debug) printf("Escribiendo %03d bytes (nodo %d) en offset %05d\n", strlen(buffer), *nw, *offset);

    /* Se posiciona el offset al final del largo escrito */
    int originalOffset = (*offset);
    (*offset) += writtenBytes;

    /* Se escriben recursivamente los nodos hijos */
    for (int i=0; i<prt->n; i++) {
        if (!isLeaf(*(prt->nodes[i]))) {
            /* Se llena el espacio de la referencia con el offset correspondiente */
            char childRef[9];
            sprintf(childRef, "%08X", (*offset));
            fseek(wrt_ptr, originalOffset + i*28 + 22, SEEK_SET);
            fwrite(childRef, 8, 1, wrt_ptr);
            if (debug) printf("originalOffset: %d, i: %d\n", originalOffset, i);
            if (debug) printf("Escribiendo referencia %s en offset %05d\n", childRef, originalOffset + i*28 + 22);
            writeTreeToBin(prt->nodes[i]->child, wrt_ptr, offset, nw, debug);
        }
    }
}

/* Lee un archivo binario de disco y retorna los datos de los nodos */
char **readBin(FILE *rd_ptr, int offset, int *readNodes, int *blocksIO, int debug) {
    char lenBuffer[2];

    /* Se lee la cantdidad de rectángulos a leer del disco */
    fseek(rd_ptr, offset, SEEK_SET);
    fread(lenBuffer, 2, 1, rd_ptr);
    

    /* Se recupera la cantidad de nodos a leer */
    char ns[3];
    int n = 0;
    sscanf(lenBuffer, "%2s", ns);
    n = (int)strtol(ns, NULL, 16);

    /* Se lee la data de los n rectángulos del disco */
    fseek(rd_ptr, offset + 2, SEEK_SET);
    char *dataBuffer = malloc(28 * n);
    fread(dataBuffer, 28*n, 1, rd_ptr);
    if (debug) printf("Data del buffer (%d bytes): %s\n", n*28, dataBuffer);

    char **data = malloc(n * sizeof(char *));
    /* Se itera por cada nodo */
    char *dataNodes = dataBuffer;
    for (int i=0; i<n; i++) {
        (*readNodes)++;
        data[i] = malloc(28 * sizeof(char) + 1);
        /* Se recuperan los datos */
        sscanf(dataNodes, "%028s", data[i]);

        /* Se avanza en el buffer */
        dataNodes += 28;
    }

    /* Se libera la memoria del buffer */
    free(dataBuffer);

    /* Se suma 1 a las operaciones IO */
    (*blocksIO)++;

    return data;
};

void printRTree(RTree rt) {
    printf("RTree con %d nodos\n", rt.n);
    
    int i;
    for (i=0; i<rt.n; i++) {
        printf("Nodo %d\n", i+1);
        printNode(*(rt.nodes[i]));
        printf("-----------------\n");
    };
};

void freeRTree(RTree *rt) {
    /* Si el nodo es interno, se libera recursivamente la memoria de los nodos hijos */
    for (int i=0; i<rt->n; i++) {
        if (!isLeaf(*(rt->nodes[i]))) {
            freeRTree(rt->nodes[i]->child);
            free(rt->nodes[i]);
        }
    }

    /* Luego, se libera la memoria reservada para los nodos */
    free(rt->nodes);
}

/* Node */
void initNode(Node *node) {
    node->child = NULL;
};

int isLeaf(Node node) {
    return node.child == NULL;
} 

void setNodeMBR(Node *node) {
    if (!isLeaf(*node)) {
        node->MBR = calcMBR(*(node->child));
    };
};

void printNode(Node node) {
    printf("MBR:");
    printRect(node.MBR);
    printf("\n");
    if (!isLeaf(node)) {
        printf("Child Tree:\n");
        printRTree(*(node.child));
    };
}

/* Rectangle */
void initRect(Rect *r, float x1, float y1, float x2, float y2) {
    /* Por definición, p1 será el que esté más abajo a la izquierda, y p2 el más arriba a la derecha */
    Point p1 = {x1<x2 ? x1 : x2, y1<y2 ? y1 : y2};
    Point p2 = {x1>x2 ? x1 : x2, y1>y2 ? y1 : y2};

    r->p1 = p1;
    r->p2 = p2;
}

Point getRectCenter(Rect r) {
    Point center;

    center.x = (r.p1.x + r.p2.x) / 2;
    center.y = (r.p1.y + r.p2.y) / 2;

    return center;
};

int areIntersecting(Rect r1, Rect r2) {
    int interX = !(r1.p2.x < r2.p1.x || r1.p1.x > r2.p2.x);
    int interY = !(r1.p2.y < r2.p1.y || r1.p1.y > r2.p2.y);
    
    return interX && interY;
}

void printRect(Rect r) {
    printPoint(r.p1);
    printf(" - ");
    printPoint(r.p2);
};

/* Point */
Point getMinPoint(Point p1, Point p2) {
    Point minPoint = {p1.x<p2.x ? p1.x : p2.x, p1.y<p2.y ? p1.y : p2.y};
    return minPoint;
};

Point getMaxPoint(Point p1, Point p2) {
    Point max = {p1.x>p2.x ? p1.x : p2.x, p1.y>p2.y ? p1.y : p2.y};
    return max;
};

void printPoint(Point p) {
    printf("(%.2f, %.2f)", p.x, p.y);
};