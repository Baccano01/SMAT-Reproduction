/// @file coo_spmv.c

/* Contains implementation for functions
 * for sparse matrix vector multiplication
 * using COO sparse matrix.
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
 * - *v, *CA, *CJ, and *CI have been allocated.
 * - *CA, *CJ, and *CI have the same number of items eq. to nnz.
 * - M is the number of rows.
 */
void coo_spmv(double **CA, int **CI, int **CJ, int nnz, int M, double **v,
              double **out) {
  if (nnz <= 0) {
    fprintf(stderr, "%s:%s:%d: 'nnz' (%d) <= 0\n", __FILE__, __func__, __LINE__,
            nnz);
    exit(EXIT_FAILURE);
  }
  if (M <= 0) {
    fprintf(stderr, "%s:%s:%d: 'M' (%d) <= 0\n", __FILE__, __func__, __LINE__,
            M);
    exit(EXIT_FAILURE);
  }

  /* verify pointers has been allocated */
  POINTER_VERIFY(v);
  POINTER_VERIFY(CA);
  POINTER_VERIFY(CI);
  POINTER_VERIFY(CJ);

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

  for (int j = 0; j < nnz; j++)
    (*out)[(*CI)[j]] += (*CA)[j] * (*v)[(*CJ)[j]];
}

#if UNIT_TEST_COO_SPMV

#include "../../../csr2coo/include/csr2coo.h"
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

  /* COO */
  double *CA;
  int *CI, *CJ, nnz;
  nnz = csr2coo(&A, &J, &I, M, &CA, &CJ, &CI);
  free(A);
  free(I);
  free(J);

  /* parses the vector */
  parse_double_vector(vect_str, &v);

  out = NULL;
  coo_spmv(&CA, &CI, &CJ, nnz, M - 1, &(v.data), &out);

  for (int i = 0; i < M - 1; i++) {
    printf(" [%lg]", out[i]);
  }
  printf("\n");

  dv_free(&v);
  free(out);
  free(CA);
  free(CJ);
  free(CI);

  return EXIT_SUCCESS;
}

#endif
