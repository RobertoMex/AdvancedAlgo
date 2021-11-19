#include<iostream>
#include "include\MatrixMath.h"
#include<chrono>
using namespace std;
using namespace std::chrono;

int max(int a, int b);

int** createArray(int row, int col);
void printMatrix(int **a, int row, int col);

int main(){

    int n = 4 ; // square matrix
    int** array1 = createArray(n,n);
    int** array2 = createArray(n,n);
    MatrixMath mm;
    printMatrix(array1,n,n);
    //Addition Test 1
    
    cout << "Addition Test 1" << endl;
    auto start = high_resolution_clock::now();
    int** array3 = mm.add2D(array1,array2,n,n);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    cout << "Addition Test Time(Milli Sec) " << duration.count() <<endl;
    //printMatrix(array3,n,n);

    //Multiply Test 1
   /*cout << "Multiply Test 1" << endl;
    auto start = high_resolution_clock::now();
    int** array4 = mm.multiply2D(array1,array2,n,n,n);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    cout << "Multiply Test Time(Milli Sec) " << duration.count() <<endl;
    */
    //printMatrix(array4,n,n);
    
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
