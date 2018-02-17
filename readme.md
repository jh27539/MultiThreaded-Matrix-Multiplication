#Multi-Threaded Matrix Multiplication

A multi-threaded console application to perform [M × N] × [N × P] 
matrix multiplication to produce an [M × P] matrix using POSIX threads (pthreads).

OBJECTIVES

Reading the two input files must be done in parallel, using two threads spawned by the main thread.
Each element of the resulted matrix is calculated by a unique thread. 
Once the multiplication is done, the parent thread (main thread) writes the resultant C matrix in the output file.

the compilation step may be the following command:
g++ -pthread main.cpp -o matrix-multiply

Useful website for calculating matrices
http://wims.unice.fr/~wims/wims.cgi

Key Concepts and Techniques

-Structures
-File Input/Ouput
-Pointers
-References
-POSIX threads 
-Concurrency
-Lists
-Prototypes
-Command Line Input Processing
