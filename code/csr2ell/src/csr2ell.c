/// @file csr2ell.c
/* This contains functions useful for
 * converting sparse matrices from CSR format
 * into ELL format.
 *
 * Copyright 2026 Jofeth Abello
 * */

#include "../include/csr2ell.h"
#include "../../convert_utils/param_verify.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/* Converts CSR format to Ellpack-Itpack (ELL) format */
int csr2ell(double **A, int **I, int **J, int M, double **COEF, int **JCOEF) {
  /* Input parameters require to be allocated pointers*/
  POINTER_VERIFY(A);
  POINTER_VERIFY(I);
  POINTER_VERIFY(J);

  /* Output parameters that require to be non-allocated pointers */
  VERIFY_NON_ALLOC(COEF);
  VERIFY_NON_ALLOC(JCOEF);

  int max_ncols = 0;
  int len;
  int i, j, k;
  /* Compute the max length of row */
  /* Find two indices i and j s.t.
   * I[i], I[j] > 0 and forall i < k < j, I[k] < 0
   * and i < j */
  i = 0;
  j = 1;
  while (i < M && j < M) {
    // if I[i], I[j] < 0 then i++, j++
    // if I[i] < 0, I[j] >= 0 then i++, j++
    // cases above have been merged in the first if statement.
    // if I[i] >= 0, I[j] < 0 then j++
    // if I[i], I[j] >= 0 then max_ncols = max(., I[j] - I[i]), i++, j++
    if ((*I)[i] < 0) {
      i++;
      j++;
    } else if ((*I)[i] >= 0 && (*I)[j] < 0) {
      j++;
    } else if ((*I)[i] >= 0 && (*I)[j] >= 0) {
      len = (*I)[j] - (*I)[i];
      max_ncols = len > max_ncols ? len : max_ncols;
      i++;
      j++;
    }
  }

  /* Reserve memory for COEF and JCOEF */
  SAFE_MALLOC(COEF, max_ncols * (M - 1));
  SAFE_MALLOC(JCOEF, max_ncols * (M - 1));

  i = 0;
  j = 1;
  while (i < M && j < M) {
    // if I[i], I[j] < 0 then i++, j++
    // if I[i] < 0, I[j] >= 0 then i++, j++
    // cases above have been merged in the first if statement.
    // if I[i] >= 0, I[j] < 0 then j++
    // if I[i], I[j] >= 0 then max_ncols = max(., I[j] - I[i]), i++, j++
    if ((*I)[i] < 0) {
      // i-th row has no nonzeros, so fill it with default values
      for (k = 0; k < max_ncols; k++) {
        (*COEF)[i * max_ncols + k] = 0;
        (*JCOEF)[i * max_ncols + k] = JCOEF_SENTINEL;
      }
      i++;
      j++;
    } else if ((*I)[i] >= 0 && (*I)[j] < 0) {
      j++;
    } else if ((*I)[i] >= 0 && (*I)[j] >= 0) {
      len = (*I)[j] - (*I)[i];

      for (k = 0; k < len; k++) {
        (*COEF)[i * max_ncols + k] = (*A)[(*I)[i] + k];
        (*JCOEF)[i * max_ncols + k] = (*J)[(*I)[i] + k];
      }
      // fill the remaining cells with zeros.
      for (k = len; k < max_ncols; k++) {
        (*COEF)[i * max_ncols + k] = 0;
        (*JCOEF)[i * max_ncols + k] = JCOEF_SENTINEL;
      }
      i++;
      j++;
    }
  }

  return max_ncols;
}

#ifdef UNIT_TEST_CSR2ELL
#include "../../mtx_mrkt_csr/include/api/csr_util.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "USAGE: ./a.out [filename] %s\n", argv[0]);
    exit(1);
  }

  double *A = NULL;
  int *I = NULL, *J = NULL, n_rows, nz;
  int i, j;

  double *COEF = NULL;
  int *JCOEF = NULL;

  n_rows = mtx_mrkt2csr(argv[1], &A, &J, &I);

  /* finds the first non-negative value in I
   * which will need to calculate the number
   * of non-zeros nz */
  for (i = 0; i < n_rows; i++) {
    if (I[i] > -1) {
      break;
    }
  }
  nz = I[n_rows - 1] - I[i];

  fprintf(stdout, "=== CSR ===\n");

  fprintf(stdout, "== I ==\n");
  for (i = 0; i < n_rows; i++) {
    fprintf(stdout, "%d ", I[i]);
  }
  fprintf(stdout, "\n");

  fprintf(stdout, "== A ==\n");
  for (i = 0; i < nz; i++) {
    fprintf(stdout, "%lg ", A[i]);
  }
  fprintf(stdout, "\n");

  fprintf(stdout, "== J ==\n");
  for (i = 0; i < nz; i++) {
    fprintf(stdout, "%d ", J[i]);
  }
  fprintf(stdout, "\n\n");

  int max_len_row = csr2ell(&A, &I, &J, n_rows, &COEF, &JCOEF);
  fprintf(stdout, "max_len_row=%d\n", max_len_row);

  fprintf(stdout, "=== ELL ===\n");
  n_rows--;
  fprintf(stdout, "== COEF ==\n");
  for (i = 0; i < n_rows; i++) {
    for (j = 0; j < max_len_row; j++) {
      fprintf(stdout, "%lg ", COEF[i * max_len_row + j]);
    }
    fprintf(stdout, "\n");
  }
  fprintf(stdout, "\n");
  fprintf(stdout, "== JCOEF ==\n");
  for (i = 0; i < n_rows; i++) {
    for (j = 0; j < max_len_row; j++) {
      fprintf(stdout, "%d ", JCOEF[i * max_len_row + j]);
    }
    fprintf(stdout, "\n");
  }
  fprintf(stdout, "\n");

  /* clean up */
  free(A);
  free(I);
  free(J);
  free(COEF);
  free(JCOEF);
  return 0;
}
#endif
