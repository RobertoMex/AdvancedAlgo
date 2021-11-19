#include "include\MatrixMath.h"
#include <omp.h>
MatrixMath::MatrixMath(){}


//Assume matrix a and b are valid to multiply
void MatrixMath::parallelMultiply2D(int** matrixA, int** matrixB, int** matrixC, int dimension){
    #pragma omp parallel for
	for(int i=0; i<dimension; i++){
		for(int j=0; j<dimension; j++){
			for(int k=0; k<dimension; k++){
				matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
			}
		}
	}
}