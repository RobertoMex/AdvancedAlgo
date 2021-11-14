#include <iostream>

#include "../Matrix.hpp"

void printSeparator()
{
    std::cout << "---------------------------" << std::endl;
}

void testOne()
{
    std::cout << "5 by 4 matrix of ones" << std::endl;
    Matrix<int, 5, 4> matrix(1);
    matrix.print();
}

void testTwo()
{
    std::cout << "3 by 6 matrix of (i-j+1)" << std::endl;
    Matrix<int, 3, 6> matrix([](std::size_t rowIdx, std::size_t colIdx)
                             { return rowIdx - colIdx + 1; });
    matrix.print();
}

void testThree()
{
    std::cout << "Sum and scalar multiply test: should print a 3 by 6 matrix of (j-i)" << std::endl;
    Matrix<int, 3, 6> matrix(1);
    Matrix<int, 3, 6> other([](std::size_t rowIdx, std::size_t colIdx)
                            { return rowIdx - colIdx + 2; });
    matrix.multiplyScalar(2);
    matrix.add(other, -1);
    matrix.print();
}

void testFour()
{
    std::cout << "Multiply left (shape): should print a 3 by 6 matrix of fours" << std::endl;
    Matrix<int, 3, 4> left(1);
    Matrix<int, 4, 6> right(1);
    Matrix<int, 3, 6> result(0);
    right.multiplyLeft<3>(left, result);
    result.print();
}

void testFive()
{
    std::cout << "Multiply right (shape): should print a 3 by 6 matrix of fours" << std::endl;
    Matrix<int, 3, 4> left(1);
    Matrix<int, 4, 6> right(1);
    Matrix<int, 3, 6> result(0);
    left.multiplyRight<6>(right, result);
    result.print();
}

int main()
{
    testOne();
    printSeparator();
    testTwo();
    printSeparator();
    testThree();
    printSeparator();
    testFour();
    printSeparator();
    testFive();

    return 0;
}
