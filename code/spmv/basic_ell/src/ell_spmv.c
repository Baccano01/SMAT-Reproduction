/// @file ell_spmv.c

/* Contains implementation for functions
 * for sparse matrix vector multiplication
 * using ELLPACK-ITPACK sparse matrix.
 *
 * Copyright 2026 Jofeth Abello
 */

#include "../../../convert_utils/param_verify.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* precond:
 * - *COEF and *JCOEF are 2D matrices in 1D arrays.
 * - M is the number of rows.
 */
void ell_spmv(double **COEF, int **JCOEF, int max_ncols, int M, double **v,
              double **out) {
  if (max_ncols <= 0) {
    fprintf(stderr, "%s:%s:%d: 'max_row_len' (%d) <= 0\n", __FILE__, __func__,
            __LINE__, max_ncols);
    exit(EXIT_FAILURE);
  }
  if (M <= 0) {
    fprintf(stderr, "%s:%s:%d: 'M' (%d) <= 0\n", __FILE__, __func__, __LINE__,
            M);
    exit(EXIT_FAILURE);
  }

  /* verify pointers has been allocated */
  POINTER_VERIFY(v);
  POINTER_VERIFY(COEF);
  POINTER_VERIFY(JCOEF);

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

  int col, row;

  for (row = 0; row < M; row++) {
    for (col = 0; col < max_ncols; col++) {
        // only zeros remain at this row, so skip to next row
        if ((*JCOEF)[row * max_ncols + col] < 0)
            break;

        (*out)[row] += (*COEF)[row * max_ncols + col] *
                        (*v)[(*JCOEF)[row * max_ncols + col]];
    }
  }
}

#if UNIT_TEST_ELL_SPMV

#include "../../../csr2ell/include/csr2ell.h"
#include "../../../mtx_mrkt_csr/include/api/csr_util.h"
#include "../../vector_parse/include/parse_vector.h"

/* special syntaxes:
 * [a@n] means create a double vector with 'n' occurences of 'a'
 * where 'n' is an integer and 'a' is a double.
 */
int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "USAGE: ./a.out [filename] \"[x1, x2, ..., xn]\"\n");
    return EXIT_FAILURE;
  }

  DoubleVec v;
  char *filename, *vect_str;

  /* CSR */
  double *A, *out;
  int *I, *J, M;

  filename = argv[1];
  vect_str = argv[2];

  /* parses the sparse matrix */
  M = mtx_mrkt2csr(filename, &A, &J, &I);

  /* ELL */
  double *COEF;
  int *JCOEF, max_ncols;
  COEF = NULL;
  JCOEF = NULL;
  max_ncols = csr2ell(&A, &I, &J, M, &COEF, &JCOEF);
  free(A);
  free(I);
  free(J);

  /* parses the vector */
  parse_double_vector(vect_str, &v);

  out = NULL;
  ell_spmv(&COEF, &JCOEF, max_ncols, M - 1, &(v.data), &out);

  double checksum = 0.0;
  for (int i = 0; i < M - 1; i++) {
        checksum += out[i];
  }
  printf("%lg\n", checksum);


#if 0
  for (int i = 0; i < M - 1; i++) {
    printf(" [%lg]", out[i]);
  }
  printf("\n");
#endif

  dv_free(&v);
  free(out);
  free(COEF);
  free(JCOEF);

  return EXIT_SUCCESS;
}

#endif
