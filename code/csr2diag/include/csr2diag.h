/// @file csr2diag.h

/* Contains API for converting sparse matrix in CSR
 * into DIAG (diagonal) format.
 *
 * Copyright 2026 Jofeth Abello
 */

/**
 * @brief Converts a sparse matrix in CSR into DIA format.
 *
 * A nonzero diagonal is a diagonal of the original matrix
 * that contains at least a nonzero.
 *
 * @param[in] **A array for nonzeros of CSR.
 * @param[in] **I array for row pointers to A and J of CSR.
 * @param[in] **J array for column indices of nonzeros of CSR.
 * @param[in] M number rows + 1 (length of I).
 * @param[in] N number cols of original sparse matrix.
 * @param[out] **DIAG 2D matrix for nonzero diagonals of DIA; 1D array.
 * @param[out] **IOFF array for nonzero diagonals' offsets relative to main
 * diagonal; increasing order.
 * @param[out] **max_diag_len the length of the longest nonzero diagonal.
 */
int csr2diag(double **A, int **I, int **J, int M, int N, double **DIAG,
             int **IOFF, int **max_diag_len);
