#ifndef TESTA_H
#define TESTA_H

class TestA
{
private:
    int year;

public:
    TestA();

    void SetYear(int y);

    int getYear() { return year; }

    int* createArray();

    void multiplyByFactor(int *arr,int factor);

};

#endif