#include "include\MatrixMath.h"

MatrixMath::MatrixMath(){}

//Assume matrix a and b have the same number of rows and cols
int** MatrixMath::add2D(int **a, int **b, int row, int col){
   int** c = 0;
   c = new int*[row]; 
   for(int i = 0; i < row; i++){
       c[i] = new int[col];
       for(int j = 0; j < col; j++){
           c[i][j] = a[i][j]  + b[i][j];
       }
   }
   return c;
}

//Assume matrix a and b are valid to multiply
int** MatrixMath::multiply2D(int **a, int **b, int rowA, int colB, int m){
    int** c = 0;
    c = new int*[rowA]; 
    for(int i = 0; i < rowA; i++){
       c[i] = new int[colB];
       for(int j = 0; j < colB; j++){
           for(int k = 0; k < m; k++){
               c[i][j] += a[i][k] * b[k][j];
           }
       }
   }
   return c;
}

