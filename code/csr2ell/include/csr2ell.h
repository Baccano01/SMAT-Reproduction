/** @file "csr2ell.h"
 * Header file containing functions and definitions for converting
 * sparse matrix in Compressed Sparse Row (CSR) format into
 * Ellpack-Itpack (ELL) format.
 * Copyright 2026 Jofeth Abello
 */
#ifndef CSR2ELL
#define CSR2ELL

#define JCOEF_SENTINEL -1

/**
 * Converts Compressed Sparse Row (CSR) sparse matrix
 * into Ellpack-Itpack (ELL) format.
 *
 * @param[in] **A pointer to the array of nonzeros for CSR
 * @param[in] **I pointer to the array of row pointers for CSR
 * @param[in] **J pointer to the array of column indices of nonzeros for CSR
 * @param[in] **COEF pointer to the matrix containing nonzeros for ELL
 * @param[in] **JCOEF pointer to the matrix containing column indices of
 * nonzeros for CSR
 * @param[out] row length of COEF and JCOEF
 */
int csr2ell(double **A, int **I, int **J, int M, double **COEF, int **JCOEF);

#endif // CSRELL
