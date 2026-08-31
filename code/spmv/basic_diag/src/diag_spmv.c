/// @file diag_spmv.c

/* Contains implementation for functions
 * for sparse matrix vector multiplication
 * using DIAG sparse matrix.
 *
 * Copyright 2026 Jofeth Abello
 */

#include <scorep/SCOREP_User.h>  // manual instrumentation
#include "../../../convert_utils/param_verify.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define max(a, b) ((a) < (b) ? (b) : (a))
#define min(a, b) ((a) > (b) ? (b) : (a))

/* precond:
 * - *DIAG and *IOFF are 2D matrices in 1D arrays.
 * - M is the number of rows.
 * - *IOFF is increasing order.
 */
void diag_spmv(double **DIAG, int **IOFF, int max_diag_len, int ndiags, int M,
               int N, double **v, double **out, int repetitions) {
  if (max_diag_len <= 0) {
    fprintf(stderr, "%s:%s:%d: 'max_diag_len' (%d) <= 0\n", __FILE__, __func__,
            __LINE__, max_diag_len);
    exit(EXIT_FAILURE);
  }
  if (ndiags <= 0) {
    fprintf(stderr, "%s:%s:%d: 'ndiags' (%d) <= 0\n", __FILE__, __func__,
            __LINE__, ndiags);
    exit(EXIT_FAILURE);
  }
  if (M <= 0) {
    fprintf(stderr, "%s:%s:%d: 'M' (%d) <= 0\n", __FILE__, __func__, __LINE__,
            M);
    exit(EXIT_FAILURE);
  }

  /* verify pointers has been allocated */
  POINTER_VERIFY(v);
  POINTER_VERIFY(DIAG);
  POINTER_VERIFY(IOFF);

  /* verify pointer has not been allocated yet */
  VERIFY_NON_ALLOC(out);

  CHECK_OVERFLOW(M / sizeof(**out) > SIZE_MAX,
                 "overflow of 'size_t' in allocation attempt",
                 exit(EXIT_FAILURE));

  /* allocate memory to output */
  *out = (double *)calloc(M, sizeof(**out));
  if (out == NULL) {
    fprintf(stderr, "%s:%s:%d: failed to allocate memory!\n", __FILE__,
            __func__, __LINE__);
    exit(EXIT_FAILURE);
  }

  /* Invariant 1: upper (or main) diagonals
   * have non-negative diagonal offsets w.r.t. main diagonal,
   * while lower diagonals have negative diagonal offsets.
   *
   * Invariant 2: diagonals with non-negative offsets always
   * start from first row of the original matrix, otherwise
   * starts from the abs(diagonal offset).
   *
   * Invariant 3: diagonals with negative offsets always
   * start from first col of the original matrix, otherwise
   * starts from diagonal offset.
   */
  int i, diag_offset, row_start, col_start;
  int diag_len, d;

  SCOREP_USER_REGION_BEGIN(
        spmv_region,
        "DIA SpMV",
        SCOREP_USER_REGION_TYPE_COMMON
  );

  for (int r = 0; r < repetitions; r++) {
      for (i = 0; i < ndiags; i++) {
        diag_offset = (*IOFF)[i];

        /* if lower diagonal then row_start is > 0 based
         * on Invariant 2 */
        row_start = max(0, -diag_offset);

        /* if upper diagonal then col_start is > 0 based
         * on Invariant 3 */
        col_start = max(0, diag_offset);
        diag_len = min(M - row_start, N - col_start);
        for (d = 0; d < diag_len; d++)
          (*out)[row_start + d] += (*DIAG)[d * ndiags + i] * (*v)[col_start + d];
      }
  }

  SCOREP_USER_REGION_END(spmv_region);
}

#if UNIT_TEST_DIAG_SPMV

#include "../../../csr2diag/include/csr2diag.h"
#include "../../../mtx_mrkt_csr/include/api/csr_util.h"
#include "../../../mtx_mrkt_csr/include/matrix_market/util.h"
#include "../../vector_parse/include/parse_vector.h"

/* special syntaxes:
 * [a@n] means create a double vector with 'n' occurences of 'a'
 * where 'n' is an integer and 'a' is a double.
 */
int main(int argc, char *argv[]) {
  if (argc < 4) {
    fprintf(stderr, "USAGE: ./a.out [filename] \"[x1, x2, ..., xn]\" [repetitions]\n");
    return EXIT_FAILURE;
  }

  DoubleVec v;
  char *filename, *vect_str;
  int input_repetitions;

  /* CSR */
  double *A, *out;
  int *I, *J, M, N;

  filename = argv[1];
  vect_str = argv[2];
  input_repetitions = argv[3];

  /* parses the sparse matrix */
  M = mtx_mrkt2csr(filename, &A, &J, &I);

  MM_DATA *mm_data;
  mm_data = read_mtx_data(filename);
  N = get_col_count(mm_data);

  /* DIAG */
  double *DIAG;
  int *IOFF, *max_diag_len, ndiags;
  DIAG = NULL;
  IOFF = NULL;
  ndiags = csr2diag(&A, &I, &J, M, N, &DIAG, &IOFF, &max_diag_len);

  free(A);
  free(I);
  free(J);

  /* parses the vector */
  parse_double_vector(vect_str, &v);

  /* WARM-UP */
  int repetitions;
  repetitions = 1;
  out = NULL;

  diag_spmv(&DIAG, &IOFF, *max_diag_len, ndiags, M - 1,
            N, &(v.data), &out, repetitions);
  free(out);

  /* MEASURE WITH REPETITION */
  repetitions = input_repetitions;
  out = NULL;
  diag_spmv(&DIAG, &IOFF, *max_diag_len, ndiags, M - 1,
            N, &(v.data), &out, repetitions);

#if 0
  for (int i = 0; i < M - 1; i++) {
    printf(" [%lg]", out[i]);
  }
  printf("\n");
#endif

  free(out);

  dv_free(&v);
  free(max_diag_len);
  free(DIAG);
  free(IOFF);
  free(mm_data);

  return EXIT_SUCCESS;
}

#endif
