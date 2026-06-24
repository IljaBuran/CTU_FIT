@echo off

i686-w64-mingw32-g++ -std=c++20 -Wall -pedantic solution.cpp sample_tester.cpp libprogtest_solver.a -o solution.exe