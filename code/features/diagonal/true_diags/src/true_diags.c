/**
 * This contains function(s) for calculating the number of "true" diagonals
 * of a sparse matrix. A "true" diagonal based on the definition at
 * https://dl.acm.org/doi/epdf/10.1145/2499370.2462181.
 *
 * Copyright 2026 Jofeth Abello
 */

#include "../../../../csr2diag/include/csr2diag.h"
#include "../../../../mtx_mrkt_csr/include/api/csr_util.h"
#include "../../../../convert_utils/param_verify.h"
#include "../../../../mtx_mrkt_csr/include/matrix_market/util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define THRESHOLD 0.5

/* Computes the "true" diagonal count from a sparse matrix in DIA format */
long true_diagonals(double **DIAG, int ndiags, long max_diag_len) {
    POINTER_VERIFY(DIAG);

    if (ndiags <= 0) {
        fprintf(stderr, "%s:%s:%d: 'ndiags' must be > 0 [%s].\n",
                __FILE__, __func__,
                __LINE__, strerror(EINVAL));
        exit(EXIT_FAILURE);
    }

    if (max_diag_len <= 0) {
        fprintf(stderr, "%s:%s:%d: 'max_diag_len' must be > 0 [%s].\n",
                __FILE__, __func__,
                __LINE__, strerror(EINVAL));
        exit(EXIT_FAILURE);
    }

    long diag_nnz;
    long true_diags;
    int row, col;

    true_diags = 0;
    for (col = 0; col < ndiags; col++) {
        diag_nnz = 0;

        for (row = 0; row < max_diag_len; row++) {
            diag_nnz += (*DIAG)[row * ndiags + col] == 0 ? 1 : 0;
        }

        if (((float) diag_nnz / max_diag_len) > THRESHOLD)
            true_diags++;
    }

    return true_diags;
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "USAGE: ./a.out [filename]\n");
        return EXIT_FAILURE;
    }

    char *filename = argv[1];

    double *A = NULL;
    int *J = NULL, *I = NULL;
    int i, rows_count, nnz;

    rows_count = mtx_mrkt2csr(filename, &A, &J, &I);
    /* finds the first non-negative value in I
     * which will need to calculate the number
     * of non-zeros nz */
    for (i = 0; i < rows_count; i++) {
        if (I[i] > -1)
            break;
    }
    nnz = I[rows_count - 1] - I[i];

    double *DIAG = NULL;
    int *IOFF = NULL;
    int *max_diag_len, ndiags, row, diag, cols_count;
    MM_DATA *mtx_data;
    mtx_data = read_mtx_data(filename);
    cols_count = get_col_count(mtx_data);
    ndiags = csr2diag(&A, &I, &J, rows_count, cols_count,
                      &DIAG, &IOFF, &max_diag_len);

    long true_diags;
    true_diags = true_diagonals(&DIAG, ndiags, *max_diag_len);
    printf("%ld", true_diags);

    free(A);
    free(J);
    free(I);
    free(max_diag_len);
    free(DIAG);
    free(IOFF);
    free(mtx_data);

    return EXIT_SUCCESS;
}
