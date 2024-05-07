#ifndef STR_h
#define STR_h

#include "RTree.h"

int compareCentersByY(const void *r1, const void *r2);
void sortNodesByYCenter(Node **nodes, int n);
RTree generateTreeSTR(Node **nodes, int n, int M, int debug);

#endif