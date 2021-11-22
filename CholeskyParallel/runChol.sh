#!/bin/bash

g++ -Wall -std=c++17 -pthread -o cholesky ./CholeskyParallel.cpp
./cholesky
rm ./cholesky
