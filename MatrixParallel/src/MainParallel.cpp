#include<stdio.h> 
#include<iostream>
#include "include\MatrixMath.h"
#include<chrono>
using namespace std;
using namespace std::chrono;

int** createArray(int row, int col);
int** createZeroArray(int row,int col);
void printMatrix(int **a, int row, int col);
//Example From: 
//https://medium.com/swlh/introduction-to-the-openmp-with-c-and-some-integrals-approximation-a7f03e9ebb65
//
int main() { 

    int n = 2500 ; // square matrix
    int** array1 = createArray(n,n);
    int** array2 = createArray(n,n);
    int** array3 = createZeroArray(n,n);
    MatrixMath mm;

    auto start = high_resolution_clock::now();
    //parallelMultiply(array1, array2, array3, n);
    mm.parallelMultiply2D(array1,array2,array3,n);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    cout << "Mutiply Test Time(Milli Sec) " << duration.count() <<endl;
    //printMatrix(array3,n,n);
        
    return 0; 
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