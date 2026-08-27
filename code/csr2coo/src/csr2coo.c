/** This contains function(s) used to convert a sparse matrix
 * from compressed sparse row (CSR) format into coordinate (COO) format.
 *
 * Copyright 2026 Jofeth Abello */

#include "../../convert_utils/param_verify.h"
#include <stdio.h>
#include <stdlib.h>

/* converts a sparse matrix in CSR into COO format. */
int csr2coo(double **A, int **J, int **I, int M, double **CA, int **CJ,
            int **CI) {
  POINTER_VERIFY(A);
  POINTER_VERIFY(J);
  POINTER_VERIFY(I);

  if (M <= 0) {
    fprintf(stderr, "%s:%s:%d Expected: M > 0, got M=%d instead!\n", __FILE__,
            __func__, __LINE__, M);
    exit(EXIT_FAILURE);
  }

  int i, j;

  /* finds the first non-negative value in I
   * which will need to calculate the number
   * of non-zeros nz */
  for (i = 0; i < M; i++) {
    if ((*I)[i] > -1)
      break;
  }
  int nz = (*I)[M - 1] - (*I)[i];

  /* reserve memory */
  SAFE_MALLOC(CA, nz);
  SAFE_MALLOC(CJ, nz);
  SAFE_MALLOC(CI, nz);

  /* i<M-1 because i indicates the rows
   * and M = #rows + 1.
   *
   * j<M because with (*I)[i], ... (*I)[j] we
   * have the window for calculating the number of nonzeros
   * in each row, and (*I)[M-1] = nz + 1 by def. of CSR */
  for (i = 0; i < M - 1; i++) {
    if ((*I)[i] > -1) {
      j = i + 1;
      while (j < M && (*I)[j] < 0)
        j++;
      for (int k = (*I)[i]; k < (*I)[j]; k++) {
        (*CA)[k] = (*A)[k];
        (*CJ)[k] = (*J)[k];
        (*CI)[k] = i;
      }
    }
  }

  return nz;
}

#ifdef UNIT_TEST_CSR2COO
#include "../../mtx_mrkt_csr/include/api/csr_util.h"
#include <assert.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "USAGE: a.out [filename] %s\n", argv[0]);
    exit(1);
  }

  double *A = NULL;
  int *I = NULL, *J = NULL, n_rows, nz;
  int i, coo_nz;

  double *CA = NULL;
  int *CI = NULL, *CJ = NULL;

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
  fprintf(stdout, "\n");

  coo_nz = csr2coo(&A, &J, &I, n_rows, &CA, &CJ, &CI);
  assert(nz == coo_nz && "CSR nz not equals to COO nz!\n");
  fprintf(stdout, "=== COO ===\n");

  fprintf(stdout, "== CA ==\n");
  for (i = 0; i < coo_nz; i++) {
    fprintf(stdout, "%lg ", CA[i]);
  }
  fprintf(stdout, "\n");
  fprintf(stdout, "== CI ==\n");
  for (i = 0; i < coo_nz; i++) {
    fprintf(stdout, "%d ", CI[i]);
  }
  fprintf(stdout, "\n");
  fprintf(stdout, "== CJ ==\n");
  for (i = 0; i < coo_nz; i++) {
    fprintf(stdout, "%d ", CJ[i]);
  }
  fprintf(stdout, "\n");

  free(A);
  free(J);
  free(I);

  free(CA);
  free(CI);
  free(CJ);

  return 0;
}

#endif
