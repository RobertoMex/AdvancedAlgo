#include<iostream>
#include "include\TestA.h"
using namespace std;

int max(int a, int b);

int* createArray();

void multiplyByFactor(int *arr, int factor);

int main(){
    cout << max(10,2) << endl;
    int *thisArray;
    TestA ta;
    thisArray = ta.createArray();
    int len = sizeof(thisArray)+1;
    cout << "Size of array " << len << endl;
    for(int i = 0; i < len; i++){
        cout << thisArray[i];
    }
    cout << endl;
    cout << "Update array" << endl;
    ta.multiplyByFactor(thisArray,2);
    for(int i = 0; i < len; i++){
        cout << thisArray[i];
    }

    return 0;
}

int max(int a, int b){
    return a>b ? a : b;
}

int* createArray(){
    static int arr[5] = {1,2,3,4,5};
    return &arr[0];
}

void multiplyByFactor(int *arr,int factor){
    int len = sizeof(arr)+1;
    cout << endl;
    for(int i = 0; i < len; i++){
        arr[i] = arr[i]*factor;
    }
}

