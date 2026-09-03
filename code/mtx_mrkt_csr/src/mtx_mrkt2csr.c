/*
 *   NOTES:
 *
 *   1) Matrix Market files are always 1-based, i.e. the index of the first
 *      element of a matrix is (1,1), not (0,0) as in C.  ADJUST THESE
 *      OFFSETS ACCORDINGLY offsets accordingly when reading and writing
 *      to files.
 *
 *   Author: Jofeth Abello
 *   Date: 2026-07-28
 */

#include "../include/matrix_market/mmio.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct entry {
  int x;
  int y;
  double val;
} entry;

// used in qsort as comparator
int comp(const void *a, const void *b) {
  /* order by row then by column*/

  int diff = ((entry *)a)->y - ((entry *)b)->y;
  if (diff != 0) {
    return diff;
  }
  return ((entry *)a)->x - ((entry *)b)->x;
}

/* reads a matrix market format sparse matrix and allocates
 * the arrays A, J, I for Compressed Sparsed Rows (CSR) format. */
int mtx_mrkt2csr(char *mtx_mrkt_path, double **A, int **J, int **I) {
  int ret_code;
  MM_typecode matcode;
  FILE *f;
  int i, M, N, nz, old_nz;
  entry *COO;
  int nz_diags = 0;

  if (mtx_mrkt_path != NULL) {
    if ((f = fopen(mtx_mrkt_path, "r")) == NULL)
      exit(EXIT_FAILURE);
  }

  if ((ret_code = mm_read_banner(f, &matcode)) != 0) {
    fprintf(stderr, "%s:%s:%d: %s\n", __FILE__, __func__, __LINE__,
           mm_strerror(ret_code));
    fprintf(stderr, "Could not process Matrix Market banner.\n");
    exit(EXIT_FAILURE);
  }

  /*  This is how one can screen matrix types if their application */
  /*  only supports a subset of the Matrix Market data types.      */

  int IS_UNSUPPORTED_TYPE =
      (mm_is_complex(matcode) || mm_is_pattern(matcode) ||
        mm_is_array(matcode) || mm_is_dense(matcode));
  if (IS_UNSUPPORTED_TYPE) {
    fprintf(stderr, "%s:%s:%d: %s not supported\n", __FILE__, __func__,
            __LINE__, mtx_mrkt_path);
    fprintf(stderr, "Sorry, this application does not support ");
    fprintf(stderr, "Market Market type: [%s]\n", mm_typecode_to_str(matcode));
    exit(EXIT_FAILURE);
  }

  /* find out size of sparse matrix .... */

  if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &nz)) != 0) {
    fprintf(stderr, "%s:%s:%d: %s\n", __FILE__, __func__, __LINE__,
           mm_strerror(ret_code));
    fprintf(stderr, "Could not process Matrix Market size.\n");
    exit(EXIT_FAILURE);
  }

  /* need to interpret 'nonzeros' if there is symmetry involved */

  /* nonzeros is the number of nonzero entries in the matrix
   *  (for general symmetry), or the number of nonzero entries on or
   *  below the diagonal (for symmetric or Hermitian symmetry), or the number
   *  of nonzero entries below the diagonal (for skew-symmetric symmetry). */

  /* need to count the number of main diagonal elements first
   * before we can allocate memory for the matrix, because of symmetry. */

  if (nz == 0) {
    fprintf(stderr, "%s:%s:%d: 'nnz' is 0\n", __FILE__, __func__, __LINE__);
    exit(EXIT_FAILURE);
  }

  old_nz = nz;
  if (!mm_is_general(matcode)) {
    if (mm_is_symmetric(matcode)) {
      int x, y;
      double v;
      int64_t last_pos = ftell(f);
      /* count the number of nonzeros in main diagonal
       * to subtract it from the number of entries
       * that have a symmetric counterpart */
      for (i = 0; i < nz; i++) {
        fscanf(f, "%d %d %lg\n", &x, &y, &v);
        nz_diags += x == y ? 1 : 0;
      }
      fseek(f, last_pos, SEEK_SET);
      nz = (nz - nz_diags) * 2 + nz_diags;
    } else if (mm_is_skew(matcode)) {
      nz = nz * 2;
    }
  }

  /* reserve memory for matrices */
  /* use Coordinate format with struct 'entry'
   * to be able to have an array where each
   * cell is the triple of value, row index,
   * and column index. Useful for performing
   * quicksort to order entries by row
   * then by column if two entries are in equals
   * in row. */
  COO = (entry *)malloc(nz * sizeof(entry));
  if (COO == NULL) {
    fprintf(stderr, "failed to allocate memory for matrix.\n");
    exit(EXIT_FAILURE);
  }

  /* NOTE: when reading in doubles, ANSI C requires the use of the "l"  */
  /*   specifier as in "%lg", "%lf", "%le", otherwise errors will occur */
  /*  (ANSI C X3.159-1989, Sec. 4.9.6.2, p. 136 lines 13-15)            */
  int j = 0;
  for (i = 0; i < old_nz; i++) {
    fscanf(f, "%d %d %lg\n", &(COO[i].y), &(COO[i].x), &(COO[i].val));

    /* adjust from base 1 to base 0 */
    COO[i].y--;
    COO[i].x--;

    if (!mm_is_general(matcode) && COO[i].y != COO[i].x) {
      COO[old_nz + j].y = COO[i].x;
      COO[old_nz + j].x = COO[i].y;
      COO[old_nz + j].val = COO[i].val;
      j++;
    }
  }

  if (f != stdin)
    fclose(f);

  /* convert to CSR*/
  qsort(COO, nz, sizeof(entry), comp);

  /* create CSR format
   * A contains the nonzeros.
   * J contains the corresponding column indices in the original matrix
   * I[k] contains the A and J indices of the start of the k-th row
   * in the original matrix. */
  *A = malloc(nz * sizeof(**A));
  if (A == NULL) {
    fprintf(stderr, "failed to allocate memory CSR 'A' array.\n");
    exit(EXIT_FAILURE);
  }
  *J = malloc(nz * sizeof(**J));
  if (J == NULL) {
    fprintf(stderr, "failed to allocate memory CSR 'J' array.\n");
    exit(EXIT_FAILURE);
  }
  *I = (int *)malloc((M + 1) * sizeof(**I));
  if (I == NULL) {
    fprintf(stderr, "failed to allocate memory CSR 'I' array.\n");
    exit(EXIT_FAILURE);
  }

  /* if k-th row only contain zeros in the original matrix
   * then I[k] = -1, otherwise I[k] = i, where i is the
   * first index in A and J that contains the first nonzero
   * of the k-th row.
   *
   * this makes it easy to determine whether a row
   * contains nonzeros.
   * */

  for (i = 0; i < M; i++)
    (*I)[i] = -1;

  int prev_row_idx = -1;

  /* populate CSR arrays */
  for (i = 0; i < nz; i++) {
    if (prev_row_idx != COO[i].y) {
      (*I)[COO[i].y] = i;
      prev_row_idx = COO[i].y;
    }
    (*A)[i] = COO[i].val;
    (*J)[i] = COO[i].x;
  }
  /* useful to have each difference of successive elements of I
   * indicate the number of elements for the corresponding row */
  (*I)[M] = nz;

  /* clean up */
  free(COO);

  return M + 1;
}

#ifdef UNIT_TEST_MTX_CSR
/*
 * USAGE: a.out [filename] > output
 * */

#include "../include/api/csr_util.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "USAGE: a.out [filename] %s\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  double *A = NULL;
  int *I = NULL, *J = NULL, n_rows, nz;
  int i;

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

  free(A);
  free(J);
  free(I);

  return 0;
}

#endif
