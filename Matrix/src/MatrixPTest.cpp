#include<stdio.h> 
#include <omp.h>
#include<iostream>
#include "include\Matrix.hpp"
#include<chrono>
using namespace std;
using namespace std::chrono;

int** createArray(int row, int col);
void parallelMultiply(int** matrixA, int** matrixB, int** matrixC, int dimension);
void printMatrix(int **a, int row, int col);
int** createZeroArray(int row,int col);
void multiply2D(int **a, int **b, int **c,int dimension);


int main() { 
   /* const Matrix<int, 4, 4> matrix([](std::size_t rowIdx, std::size_t colIdx)
                                   { return rowIdx + 1; });
    matrix.print();
    */
    int n = 4 ; // square matrix
    int** array1 = createArray(n,n);
    int** array2 = createArray(n,n);
    int** array3 = createZeroArray(n,n);

    auto start = high_resolution_clock::now();
    //parallelMultiply(array1, array2, array3, n);
    //multiply2D(array1,array2,array3,n);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    cout << "Mutiply Test Time(Milli Sec) " << duration.count() <<endl;
    printMatrix(array3,n,n);
        
    return 0; 
}

void parallelMultiply(int** matrixA, int** matrixB, int** matrixC, int dimension){

	#pragma omp parallel for
	for(int i=0; i<dimension; i++){
		for(int j=0; j<dimension; j++){
			for(int k=0; k<dimension; k++){
				matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
			}
		}
	}

}

void multiply2D(int **a, int **b, int **c,int dimension){
    for(int i=0; i<dimension; i++){
		for(int j=0; j<dimension; j++){
			for(int k=0; k<dimension; k++){
				c[i][j] += c[i][k] * c[k][j];
			}
		}
	}
}

int** createArray(int row, int col){
    int** arr = 0;
    arr = new int*[row];
    for(int i = 0; i < row; i++){
        arr[i] = new int[col];
        for(int j = 0; j < col ; j++){
            arr[i][j] = i+1;
        }
    }
    return arr; 
}

int** createZeroArray(int row,int col){
    int** arr = 0;
    arr = new int*[row];
    for(int i = 0; i < row; i++){
        arr[i] = new int[col];
        for(int j = 0; j < col ; j++){
            arr[i][j] = 0;
        }
    }
    return arr; 
}

void printMatrix(int **a, int row, int col){

    cout << "int print Matrix" << endl;
    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){
            cout << a[i][j];
            if(j + 1 != col){
                cout << " , ";
            }
        }
        cout << endl;
    }
}