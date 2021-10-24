#include "include\TestA.h"

TestA::TestA(){}

void TestA::SetYear(int y){
    year = y;
}

int* TestA::createArray(){
    static int arr[5] = {1,2,3,4,5};
    return &arr[0];
}

void TestA::multiplyByFactor(int *arr,int factor){
    int len = sizeof(arr)+1;
    for(int i = 0; i < len; i++){
        arr[i] = arr[i]*factor;
    }
}