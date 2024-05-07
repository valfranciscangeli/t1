#ifndef RTree_h
#define RTree_h
#include <stdio.h>

typedef struct Point {
    float x;
    float y;
} Point;

typedef struct Rect {
    Point p1;
    Point p2;
} Rect;

struct RTree;

typedef struct Node {
    Rect MBR;
    struct RTree *child;
} Node;

typedef struct RTree {
    Node **nodes;
    int n;
} RTree;

void writeTreeToBin(RTree *prt, FILE *wrt_ptr, int *offset, int *nw, int debug);
char **readBin(FILE *rd_ptr, int offset, int *readNodes, int *blocksIO, int debug);
void searchRect(FILE *rd_ptr, int offset, Rect r, int *blocksIO, int *count, Rect **resultRects, int debug);

/* RTree */
void initRTree(RTree *rt, int size);
void addNode(RTree *rt, Node *node);
Rect calcMBR(RTree rt);
int treeHeight(RTree *prt);
int isValidTree(RTree *prt);
void printRTree(RTree rt);
void freeRTree(RTree *rt);

/* Node */
void initNode(Node *node);
int isLeaf(Node node);
void setNodeMBR(Node *node);
void printNode(Node node);

/* Rectangle */
void initRect(Rect *r, float x1, float y1, float x2, float y2);
Point getRectCenter(Rect r);
int areIntersecting(Rect r1, Rect r2);
void printRect(Rect r);

/* Point */
Point getMinPoint(Point p1, Point p2);
Point getMaxPoint(Point p1, Point p2);
void printPoint(Point p);

#endif