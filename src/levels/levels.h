#ifndef LEVELS_H
#define LEVELS_H

typedef struct {
    int rows;
    int cols;
    const int *data;
		
} Matrix;

extern const Matrix levels[];

#endif