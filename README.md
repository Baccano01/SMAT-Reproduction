# SMAT-Reimplementation

This repo contains my reimplementation without the original source code of the SMAT autotuner [https://dl.acm.org/doi/10.1145/2499370.2462181](J. Li et al., PLDI 2013).
This autotuner dynamically selects the sparse matrix format (COO, CSR, ELL, DIA) to use during sparse matrix-vector multiplication. The kernels are written entirely in C.

Uses a subset of [https://sparse.tamu.edu/](SuiteSparse Matrix Collection): 1285 matrices, each with at most 10K rows and columns.

**Current still in progress: writing optimized kernels and model training in progress.**

## How it works
Given a sparse matrix in CSR format, SMAT will automatically calculate the features of that matrix (e.g. number of diagonals, nonzeros per row, ...) and then select using a ML model an optimal sparse format to be used for the sparse matrix-vector multiplication (SpMv). Each format has an associated optimized kernel on AMD EPYC 7301 (16-Core).

**Notable feature**: This can be easily extended to be optimal to other CPUs by substituting the optimized kernels specific for the desired CPU model. Making it architecture-independent.
- The sparse matrix format accepted is the Matrix Market format.
- It accepts COO matrix format which will be converted into CSR.

## How to build
In progress..., but you can build using the Makefiles in each _csr2format_ directory, _mtx_mrkt_csr_ directory, and each directory in _spmv_.

## Features Database
The features can be extracted using the scripts in _features_ directory.
