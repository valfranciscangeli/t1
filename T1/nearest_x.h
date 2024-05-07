#ifndef nearest_x_h
#define nearest_x_h

#include "RTree.h"

int compareCentersByX(const void *r1, const void *r2);
void sortNodesByXCenter(Node **nodes, int n);
RTree generateTreeNearestX(Node **nodes, int n, int M, int debug);

#endif