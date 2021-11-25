#include <iostream>
#include <chrono>
#include <omp.h>
#include "include\Matrix.hpp"
using namespace std;
using namespace std::chrono;

int** createUpperTriangleMatrix(int row, int col);
int** createRightHandSide(int row);
void printMatrix(int **a, int row, int col);
void printMatrixDouble(double **a, int row, int col);
double** resultArray(int row);
int** createRightHandSide2(int row);

void backSubstitutionSeq(int** m, int** rhs, double** result,int mRow,int mCol);
void backSubstitutionParallel(int** m, int** rhs, double** result, int mRow);

int main(){
    int n = 20000; 
    int** m = createUpperTriangleMatrix(n,n);
    int** rhs = createRightHandSide2(n);
    double** result = resultArray(n);
    

    auto start = high_resolution_clock::now();
    //backSubstitutionSeq(m,rhs,result,n,n);
    backSubstitutionParallel(m,rhs,result,n);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    cout << "Back Sub Parallel ms " << duration.count() <<endl;


    /*
    backSubstitutionParallel2(m,rhs,result,n,n);
    printMatrix(m,n,n);
    cout  << endl;
    printMatrix(rhs,n,1);
    cout << "Result" << endl;
    printMatrixDouble(result,n,1);  
    */
}


void backSubstitutionParallel(int** m, int** rhs, double** result, int mRow){
    int i,j;
    for(int v=0; v < mRow; v++){
        result[v][0] = rhs[v][0];
    }      
    for(i = mRow -1; i > 0; i--){
        #pragma omp parallel shared(m,result) private(j)
        {
            #pragma omp for
            for(j = i-1; j > -1; j--){
                result[j][0] -= result[i][0]*m[j][i];
            }
        }
        result[i][0] = result[i][0] / m[i][i];
    }
}

void backSubstitutionSeq(int** m, int** rhs, double** result,int mRow,int mCol){
    for(int i = mRow -1; i > -1; i--){
        double sumPrev = 0;
        for(int j = mCol - 1; j > i; j--){
            sumPrev += result[j][0]*m[i][j];
        }
        result[i][0] = (rhs[i][0] - sumPrev)/m[i][i];
    }
}


int** createUpperTriangleMatrix(int row, int col){
    int** arr = 0;
    arr = new int*[row];
    int value = 1;
    for(int i = 0; i < row; i++){
        arr[i] = new int[col];
        for(int j = 0; j < col ; j++){
            if(i <= j){
                arr[i][j] = value;
                value++;
            }else{
                arr[i][j] = 0;
            }
        }
        value = 1;
    }
    return arr; 
}

int** createRightHandSide(int row){
    int** arr = 0;
    arr = new int*[row];
    int value = row;
    for(int i = 0; i < row; i++){
        arr[i] = new int[1];
        arr[i][0] = value;
        value--;
    }
    return arr; 
}

int** createRightHandSide2(int row){
    int** arr = 0;
    arr = new int*[row];
    int value = 1;
    for(int i = 0; i < row; i++){
        arr[i] = new int[1];
        arr[i][0] = value;
        value++;
    }
    return arr; 
}

double** resultArray(int row){
    double** arr = 0;
    arr = new double*[row];
    for(int i = 0; i < row; i++){
        arr[i] = new double[1];
        arr[i][0] = 0.0;
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

void printMatrixDouble(double **a, int row, int col){
    cout << "Double print Matrix" << endl;
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