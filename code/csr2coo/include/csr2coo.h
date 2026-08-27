/** @file csr2coo.h
 * Contains functions useful
 * for converting Compressed Sparse Row (CSR)
 * format into Coordinate (COO) format.
 *
 * Copyright 2026 Jofeth Abello
 */

#ifndef CSR2COO
#define CSR2COO

/** Converts a CSR sparse matrix into a COO format.
 *
 * @param[in] **A pointer to the A array of CSR
 * @param[in] **J pointer to the J array of CSR
 * @param[in] **I pointer to the I array of CSR
 * @param[in] M number of rows in the original sparse matrix.
 * @param[out] **CA pointer to the A array of COO
 * @param[out] **CJ pointer to the J array of COO
 * @param[out] **CI pointer to the I array of COO
 */
int csr2coo(double **A, int **J, int **I, int M, double **CA, int **CJ,
            int **CI);

#endif // CSR2COO
