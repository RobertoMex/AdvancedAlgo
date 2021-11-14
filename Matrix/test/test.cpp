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

void testSix()
{
    std::cout << "Multiply left (value): should print an 8 by 8 matrix of (i-j) with last row equal to zero" << std::endl;
    Matrix<int, 8, 8> left([](std::size_t rowIdx, std::size_t colIdx)
                           { return (colIdx == rowIdx + 1) ? 1 : 0; });
    Matrix<int, 8, 8> right([](std::size_t rowIdx, std::size_t colIdx)
                            { return rowIdx - colIdx - 1; });
    Matrix<int, 8, 8> result(0);
    right.multiplyLeft<8>(left, result);
    result.print();
}

void testSeven()
{
    std::cout << "Multiply right (value): should print an 8 by 8 matrix of (i-j) with last row equal to zero" << std::endl;
    Matrix<int, 8, 8> left([](std::size_t rowIdx, std::size_t colIdx)
                           { return (colIdx == rowIdx + 1) ? 1 : 0; });
    Matrix<int, 8, 8> right([](std::size_t rowIdx, std::size_t colIdx)
                            { return rowIdx - colIdx - 1; });
    Matrix<int, 8, 8> result(0);
    left.multiplyRight<8>(right, result);
    result.print();
}

void testEight()
{
    std::cout << "Transpose: should print a 3 by 6 matrix of (i-j+1)" << std::endl;
    Matrix<int, 6, 3> matrix([](std::size_t rowIdx, std::size_t colIdx)
                             { return colIdx - rowIdx + 1; });
    Matrix<int, 3, 6> transpose = matrix.transpose();
    transpose.print();
}

void testNine()
{
    std::cout << "Submatrix: should print an 8 by 8 matrix of (i-j+1)" << std::endl;
    Matrix<int, 12, 12> matrix([](std::size_t rowIdx, std::size_t colIdx)
                               { return rowIdx - colIdx; });
    Matrix<int, 8, 8> submatrix = matrix.submatrix<8, 8>(3, 2);
    submatrix.print();
}

void testTen()
{
    std::cout << "Overwrite submatrix: should print a 12 by 12 matrix of ones with a lower-left block of (i-j) (in submatrix indices)" << std::endl;
    Matrix<int, 12, 12> matrix(1);
    Matrix<int, 6, 6> submatrix([](std::size_t rowIdx, std::size_t colIdx)
                                { return rowIdx - colIdx; });
    matrix.overwriteSubmatrix<6, 6>(submatrix, 6, 0);
    matrix.print();
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
    printSeparator();
    testSix();
    printSeparator();
    testSeven();
    printSeparator();
    testEight();
    printSeparator();
    testNine();
    printSeparator();
    testTen();

    return 0;
}
