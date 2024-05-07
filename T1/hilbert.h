#ifndef hilbert_h
#define hilbert_h

#include "RTree.h"

void rot(unsigned long long int n, unsigned long long int *x, unsigned long long int *y, unsigned long long int rx, unsigned long long int ry);
unsigned long long int xy2d(unsigned long long int x, unsigned long long int y);
int compareCentersByHilbert(const void *r1, const void *r2);
void sortNodesByHilbert(Node **nodes, int n);
RTree generateTreeHilbert(Node **nodes, int n, int M, int debug);

#endif