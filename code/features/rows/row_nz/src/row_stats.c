/* This source file contains functions for computing
 * nonzero distribution on rows for sparse matrices
 * in Matrix Market format.
 *
 * Copyright 2026 Jofeth Abello
 */

#include "../../../../mtx_mrkt_csr/include/api/csr_util.h"
#include "../../../../convert_utils/param_verify.h"
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define SENTINEL -1
#define SQUARE(x) ((x) * (x))


double aver_RD(double nnz, double rows_count) {
    if (nnz <= 0) {
        fprintf(stderr, "%s:%s:%d: 'nnz' must be > 0\n",
                __FILE__, __func__, __LINE__);
        exit(EXIT_FAILURE);
    }
    if (rows_count <= 0) {
        fprintf(stderr, "%s:%s:%d: 'rows_count' must be > 0\n",
                __FILE__, __func__, __LINE__);
        exit(EXIT_FAILURE);
    }
    return nnz / rows_count;
}

/* M = #rows + 1 (i.e. the length of I)
 * nz_x_row is an output parameter */
long nz_rows(double **A, int **I, int M, long **nz_x_row) {
    POINTER_VERIFY(A);
    POINTER_VERIFY(I);

    VERIFY_NON_ALLOC(nz_x_row);

    long nzrow_count;
    int i, j, k, end;

    *nz_x_row = (long *) calloc(M, sizeof(**nz_x_row));
    if (*nz_x_row == NULL) {
        free(A);
        free(I);
        fprintf(stderr, "%s:%s:%d: Failed to allocate memory for 'nz_x_row'\n",
                __FILE__, __func__, __LINE__);
        exit(EXIT_FAILURE);
    }

    /* Find two indices i and j s.t.
    * I[i], I[j] > 0 and forall i < k < j, I[k] < 0
    * and i < j */
    i = 0;
    j = 1;
    nzrow_count = 0;
    while (i < M && j < M) {
        // if I[i], I[j] < 0 then i++, j++
        // if i == j then j++
        // if I[i], I[j] >= 0 then max_ncols = max(., I[j] - I[i]), i++, j++
        if ((*I)[i] < 0) {
            i++;
        } else if ((*I)[j] < 0 || i == j) {
            j++;
        } else {
            end = (*I)[j];

            for (k = (*I)[i]; k < end; k ++) {
                if ((*A)[k] != 0)
                    (*nz_x_row)[nzrow_count]++;
            }

            nzrow_count++;
            i++;
            j++;
        }
    }

    if (nzrow_count > M - 1) {
        free(nz_x_row);
        free(A);
        free(I);
        fprintf(stderr, "%s:%s:%d: 'nzrow_count' must be <= %d, got %ld\n",
                __FILE__, __func__, __LINE__, M - 1, nzrow_count);
        exit(EXIT_FAILURE);
    }
    (*nz_x_row)[nzrow_count] = SENTINEL;
    return nzrow_count;
}

long max_RD(long **nz_x_row, long nzrow_count) {
    if (nzrow_count <= 0) {
        free(nz_x_row);
        fprintf(stderr, "%s:%s:%d: 'nzrow_count' must be > 0, got %ld\n",
                __FILE__, __func__, __LINE__, nzrow_count);
        exit(EXIT_FAILURE);
    }
    POINTER_VERIFY(nz_x_row);

    long max_nnz = LONG_MIN;
    long i, v;

    for (i = 0; i < nzrow_count && (*nz_x_row)[i] != SENTINEL; i++) {
        v = (*nz_x_row)[i];
        max_nnz = v > max_nnz ? v : max_nnz;
    }

    return max_nnz;
}

double var_RD(double aver_RD, int rows_count, long **nz_x_row, long nzrow_count) {
    if (aver_RD < 0) {
        free(nz_x_row);
        fprintf(stderr, "%s:%s:%d: 'aver_RD' must be >= 0\n",
                __FILE__, __func__, __LINE__);
        exit(EXIT_FAILURE);
    }
    if (rows_count <= 0) {
        free(nz_x_row);
        fprintf(stderr, "%s:%s:%d: 'rows_count' must be > 0\n",
                __FILE__, __func__, __LINE__);
        exit(EXIT_FAILURE);
    }
    if (nzrow_count <= 0) {
        free(nz_x_row);
        fprintf(stderr, "%s:%s:%d: 'nzrow_count' must be > 0\n",
                __FILE__, __func__, __LINE__);
        exit(EXIT_FAILURE);
    }


    POINTER_VERIFY(nz_x_row);

    double ssd = 0.0;
    double diff;
    for (long i = 0; i < nzrow_count && (*nz_x_row)[i] != SENTINEL; i++) {
        diff = fabs((*nz_x_row)[i] - aver_RD);
        ssd += SQUARE(diff);
    }

    return ssd / (double) rows_count;
}

double ER_ELL(double nnz, double max_rd, double rows_count) {
    if (max_rd <= 0) {
        fprintf(stderr, "%s:%s:%d: 'max_rd' must be > 0\n",
                __FILE__, __func__, __LINE__);
        exit(EXIT_FAILURE);
    }
    if (rows_count <= 0) {
        fprintf(stderr, "%s:%s:%d: 'row_count' must be > 0\n",
                __FILE__, __func__, __LINE__);
        exit(EXIT_FAILURE);
    }

    return nnz / (max_rd * rows_count);
}



/* Usage: ./a.out [filename] */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "%s:%s:%d: Usage: ./a.out [filename]\n",
                __FILE__, __func__, __LINE__);
        return EXIT_FAILURE;
    }

    char *filename = argv[1];

    double *A = NULL;
    int *J = NULL, *I = NULL;
    int M, nnz, i;

    M = mtx_mrkt2csr(filename, &A, &J, &I);
    if (M <= 0) {
        fprintf(stderr, "%s:%s:%d: 'M' should be > 0\n",
                __FILE__, __func__, __LINE__);
        return EXIT_FAILURE;
    }

    /* finds the first non-negative value in I
     * which will need to calculate the number
     * of non-zeros nz */
    for (i = 0; i < M; i++) {
        if (I[i] > -1) {
            break;
        }
    }
    nnz = I[M - 1] - I[i];

    double aver_rd, var_rd, er_ell;
    long nzrow_count, max_rd, *nz_x_row;
    nz_x_row = NULL;

    aver_rd = aver_RD(nnz, M - 1);
    nzrow_count = nz_rows(&A, &I, M, &nz_x_row);
    max_rd = max_RD(&nz_x_row, nzrow_count);
    var_rd = var_RD(aver_rd, M - 1, &nz_x_row, nzrow_count);
    er_ell = ER_ELL(nnz, max_rd, M - 1);

    printf("%s,%ld,%lg,%lg\n", filename, max_rd, var_rd, er_ell);

#if 0
    printf("aver_rd=%lg\n", aver_rd);
    printf("nzrow_count=%ld\n", nzrow_count);
    printf("max_rd=%ld\n", max_rd);
    printf("var_rd=%lg\n", var_rd);
#endif

    free(A);
    free(I);
    free(J);
    free(nz_x_row);

    return EXIT_SUCCESS;
}
