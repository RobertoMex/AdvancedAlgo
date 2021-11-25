#!/bin/bash

g++ -Wall -std=c++17 -pthread -o backsub ./BackSubstitutionParallel.cpp
./backsub
rm ./backsub
