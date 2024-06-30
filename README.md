# DEMO

## Contents
- [Introduction](#introduction)
- [Prerequisites](#prerequisites)
- [Building on linux](#building-on-linux)
- [How to use](#how-to-use)

## Introduction
Only tested on a 64-bit platform.<br>
This is a demo to evaluate functions with FHE-based LUT.<br>
The complete README will be published after the paper is accepted.<br>
For testing, you have to change the parameters by yourself. I'm sorry for the inconvenience.

## Prerequisites
- [OpenFHE 1.1.4](https://github.com/openfheorg/openfhe-development)
- [CMake](https://cmake.org/)
- [OpenMP](https://www.openmp.org/)

## Building on Linux
OpenFHE 1.1.4, CMake, and OpenMP are needed.<br>
For a simple test, under the `~/build` directory, run the commands below to compile the source code.<br>
You can edit the `CMakeList.txt` file as needed. Once you change the code, you need to compile again.
```
cmake ..
make
```

## How to use
The LUTs are storing in `Table` file. Do not need to prepare again. (If you need to construct LUTs again, use `makeTable.py` and make the parts you need to be availabled.)

1. Currently, you need to change the row size and table path by yourself. (The codes will be updated before publish.)<br>
The paths are like below. You can find them in the code.
```
int64_t vSize = pow(2,d); // please change d to the bit lengths of input and output
...
read_table("Table/128bit/one/vectorOfInLUT_m_d.txt"); \\ please change m to the number of inputs and d to the bit lengths of input and output
```
2. To run the demo code, run the commands like below.<br>
Please change the number of thread and code as you need.
```
OMP_NUM_THREADS=1 ./oneInput_funs_pt
```
3. For the test of related works, you also need to change the `vSize` and tables path by yourself. (The codes will be updated before publish.)<br>
The paths are like below. You can find them in the code.
```
#define vSize pow(2,d) // please change d to the bit lengths of input and output
...
read_vector("Table/RelatedWork/relatedwork_in_d.txt"); \\ please change d to the bit lengths of input and output
...
```

### Code construciton
```
src  -- build
     |_ Table # store LUTs
     |_ # source codes
       |_ CMakeList.ctxt
       |_ makeTable.py # construct plaintext LUTs data
       |_ oneInput_funs_pt.cpp # one-input function eval
       |_ twoInput_funs_pt.cpp # two-input function eval
       |_ threeInput_funs_pt.cpp # three-input function eval
       |_ demo_runtime.cpp(demo_runtime2.cpp) # primitive runtime test
       |_ demo_relatedwork.cpp # related work [MMN22] test
       |_ demo_relatedwork2.cpp # related work [OCHK18] test
       |_ bitwise.cpp(bitwise_new.cpp) # naive bit-wise LUT test
```
