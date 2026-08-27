/** @file csr_util.h
 * Contains prototypes for accessing real matrix market format matrices
 * and formatting them into CSR format for SpMV.
 *
 * Copyright 2026 Jofeth Abello
 */

#ifndef CSR_UTIL
#define CSR_UTIL

/** Reads a matrix market real matrix and allocates
 * it into Compressed Sparse Row (CSR) format arrays.
 *
 * @param[in] mtx_mrkt_path path of the matrix market matrix.
 * @param[in] **A pointer of the value array of CSR.
 * @param[in] **J pointer of the column index array of CSR.
 * @param[in] **I pointer of the array containing row pointers in A and J
 * @param[out] number of elements of I (i.e. number of rows + 1).
 */
int mtx_mrkt2csr(char *mtx_mrkt_path, double **A, int **J, int **I);

/// forward declaration of an opaque pointer for those that include csr_util.h
typedef struct MM_DATA MM_DATA;

#endif // CSR_UTIL
