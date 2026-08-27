/// @file csr_spmv.c

/* Contains implementation for functions
 * for sparse matrix vector multiplication
 * using CSR sparse matrix.
 *
 * Copyright 2026 Jofeth Abello
 */

#include "../../../convert_utils/param_verify.h"
#include "../../../mtx_mrkt_csr/include/api/csr_util.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* precond:
 * - The length of v is eq to the number of cols.
 * - M is the number of rows of original matrix + 1 (i.e. length of I) */
void csr_spmv(double **A, int **I, int **J, int M, double **v, double **out) {
  int i, j;
  int start, end;

  if (M < 0) {
    fprintf(stderr, "%s:%s:%d: 'M' < 0\n", __FILE__, __func__, __LINE__);
    exit(EXIT_FAILURE);
  }

  /* verify pointers has been allocated */
  POINTER_VERIFY(v);
  POINTER_VERIFY(A);
  POINTER_VERIFY(I);
  POINTER_VERIFY(J);

  /* verify pointer has not been allocated yet */
  VERIFY_NON_ALLOC(out);

  CHECK_OVERFLOW((M - 1) / sizeof(**out) > SIZE_MAX,
                 "overflow of 'size_t' in allocation attempt",
                 exit(EXIT_FAILURE));

  /* allocate memory to output */
  *out = (double *)calloc((M - 1), sizeof(**out));
  if (out == NULL) {
    fprintf(stderr, "%s:%s:%d: failed to allocate memory!\n", __FILE__,
            __func__, __LINE__);
    exit(EXIT_FAILURE);
  }

  i = 0;
  j = 1;

  while (j < M) {
    if ((*I)[i] < 0) {
      /* i-th row has no nonzeros */
      i++;
      j++;
    } else if ((*I)[j] < 0) {
      /* i-th row has nonzeros but don't know how many */
      j++;
    } else {
      /* i-th row has nonzeros */
      end = (*I)[j];
      for (start = (*I)[i]; start < end; start++) {
        (*out)[i] += (*A)[start] * (*v)[(*J)[start]];
      }
      i++;
      j++;
    }
  }
}

#if UNIT_TEST_CSR_SPMV

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

  /* parses the vector */
  parse_double_vector(vect_str, &v);

  out = NULL;
  csr_spmv(&A, &I, &J, M, &(v.data), &out);

  for (int i = 0; i < M - 1; i++) {
    printf(" [%lg]", out[i]);
  }
  printf("\n");

  dv_free(&v);
  free(A);
  free(I);
  free(J);
  free(out);

  return EXIT_SUCCESS;
}

#endif
