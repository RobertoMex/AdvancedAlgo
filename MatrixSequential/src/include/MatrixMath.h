#ifndef MATRIXMATH_H
#define MATRIXMATH_H

class MatrixMath{


public:
    MatrixMath();

    int** add2D(int **a, int **b, int row, int col);
    int** multiply2D(int **a, int **b, int rowA, int colB, int m);

};

#endif