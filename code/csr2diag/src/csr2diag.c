/// @file csr2diag.c
/* Contains functions useful
 * for converting sparse matrix in
 * Compressed Sparse Row (CSR) into Diagonal format
 * Copyright 2026 Jofeth Abello */

#include "../../config/config.h"
#include "../../convert_utils/param_verify.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ###############################################
// Bit Array (used for checking whether a diagonal
// contains at least a nonzero in diagonal format)
// Reference: https://www.cs.emory.edu/~cheung/Courses/255/
// https://www.cs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
// ###############################################
#define VERIFY_INDEX_DOM(k)                                                    \
  do {                                                                         \
    if ((k) < 0) {                                                             \
      errno = EINVAL;                                                          \
      return EXIT_FAILURE;                                                     \
    }                                                                          \
  } while (0)

#define BYTE_WIDTH 8
#define NOT_FOUND -1

static int set_bit(unsigned int A[], int k) {
  VERIFY_INDEX_DOM(k);
  const size_t BIT_WIDTH = sizeof(*A) * BYTE_WIDTH;
  A[k / BIT_WIDTH] |= (1 << (k % BIT_WIDTH));
  return EXIT_SUCCESS;
}

/* WARNING: -1 is also evaluated as truthy, so
 * make sure to check if there was an error first
 * i.e. it returns -1 and errno is set. */
static int test_bit(unsigned int A[], int k) {
  if (k < 0) {
    errno = EINVAL;
    return -1;
  }
  const size_t BIT_WIDTH = sizeof(*A) * BYTE_WIDTH;
  return A[k / BIT_WIDTH] & (1 << (k % BIT_WIDTH));
}

static int clear_bit(unsigned int A[], int k) {
  VERIFY_INDEX_DOM(k);
  const size_t BIT_WIDTH = sizeof(*A) * BYTE_WIDTH;
  A[k / BIT_WIDTH] &= ~(1 << (k % BIT_WIDTH));
  return EXIT_SUCCESS;
}

// ###############################################

static int set_diagonal(unsigned int **bit_arr, int k) {
  errno = 0;
  int is_set = test_bit(*bit_arr, k);
  if (is_set < 0 && errno) {
    fprintf(stderr, "%s:%s:%d: failed to set bit array - %s\n", __FILE__,
            __func__, __LINE__, strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (!is_set) {
    set_bit(*bit_arr, k);
    return 1;
  }
  return 0;
}

/* sets the k-th bit of bit_arr_pos[i] to 1 if the diagonal
 * with an offset relative to the main diagonal contains
 * at least one nonzero based on the implementation described
 * above.
 *
 * returns the number of unique diagonals that contain
 * at least a nonzero */
static int set_diagonals(int max_ndiags_pos, int max_ndiags_neg,
                         unsigned int **bit_arr_pos, unsigned int **bit_arr_neg,
                         int M, int **I, int **J) {
  int i, j, k;
  int diag_offset, bit_arr_len;
  int ndiags;
  i = 0;
  j = 1;
  ndiags = 0;

  /* Since we have negative diagonal offsets,
   * we can use the symmetry of the integer number line
   * by having two bit arrays */

  /* bit array for positive offset diagonals
   * that serve as membership look-up */

  /* since each group of bits is a position */
  /* +1 otherwise only covers indices in [0, n-2] when need
   * to cover n indices */
  bit_arr_len = ceil(
        (float)max_ndiags_pos / (float)(sizeof(**bit_arr_pos) * BYTE_WIDTH)
        ) + 1;

  /* we to avoid zero allocation, if max_ndiags_pos < sizeof(int) */
  if (bit_arr_len <= 0)
    bit_arr_len = 1;
  *bit_arr_pos = calloc(bit_arr_len, sizeof(**bit_arr_pos));
  if (*bit_arr_pos == NULL) {
    fprintf(stderr, "%s:%s:%d: Could not allocate memory.\n", __FILE__,
            __func__, __LINE__);
    exit(EXIT_FAILURE);
  }

  /* bit array for negative offset diagonals
   * that serve as membership look-up */

  /* since each group of bits is a position */
  /* +1 otherwise only covers indices in [0, n-2] when need
   * to cover n indices */
  bit_arr_len = ceil(
        (float)max_ndiags_neg / (float)(sizeof(**bit_arr_neg) * BYTE_WIDTH)
        ) + 1;

  /* we to avoid zero allocation, if max_ndiags_pos < sizeof(unsigned int) */
  if (bit_arr_len <= 0)
    bit_arr_len = 1;
  *bit_arr_neg = calloc(bit_arr_len, sizeof(**bit_arr_neg));
  if (*bit_arr_neg == NULL) {
    fprintf(stderr, "%s:%s:%d: Could not allocate memory.\n", __FILE__,
            __func__, __LINE__);
    exit(EXIT_FAILURE);
  }
  while (i < M && j < M) {
    /* i-th row only has no nonzeros. */
    if ((*I)[i] < 0) {
      i++;
    } else if ((*I)[j] < 0 || i == j) {
      j++;
    } else {
      for (k = (*I)[i]; k < (*I)[j]; k++) {
        /* save the diagonals to be included
         * in the bits array */
        diag_offset = (*J)[k] - i;
        if (diag_offset >= 0)
          ndiags += set_diagonal(bit_arr_pos, diag_offset);
        else
          ndiags += set_diagonal(bit_arr_neg, abs(diag_offset));
      }
      i++;
      j++;
    }
  }
  return ndiags;
}

/* calculates the diagonal length of the diagonal
 * corresponding to the 'diag_offset' */
static int diag_len(int rows, int cols, int diag_offset) {
  int main_diag_len, res;
  /* works for both square and rectangular matrices */
  main_diag_len = cols < rows ? cols : rows;

  /* calculate the diagonal length */
  /* Square matrix case */
  if (rows == cols) {
    if (diag_offset == 0) {
      res = main_diag_len;
    } else {
      /* works for both upper and lower diagonals */
      res = main_diag_len - abs(diag_offset);
    }
  } else if (cols < rows) {
    /* Rectangular matrix case */
    /* lower diagonal */
    if (diag_offset < 0) {
      int potent_ext = rows + diag_offset;
      res = potent_ext < cols ? potent_ext : cols;
    } else if (diag_offset > 0) {
      /* upper diagonal */
      res = main_diag_len - diag_offset;
    } else {
      /* main diagonal */
      res = main_diag_len;
    }
  } else {
    /* lower diagonal */
    if (diag_offset < 0) {
      res = main_diag_len - abs(diag_offset);
    } else if (diag_offset > 0) {
      /* upper diagonal */
      int potent_ext = cols - diag_offset;
      res = potent_ext < rows ? potent_ext : rows;
    } else {
      /* main diagonal */
      res = main_diag_len;
    }
  }
  return res;
}

/* preconds:
 * - M = #rows + 1
 * - N = #cols */
static int longest_diag_len(int M, int N, int **I, int **J) {
  int i, j, k;
  int diag_offset;
  int dlen;
  int max_diag_len, rows, cols;

  rows = M - 1;
  cols = N;

  max_diag_len = INT_MIN;
  i = 0;
  j = 1;
  while (i < M && j < M) {
    /* i-th row only has no nonzeros. */
    if ((*I)[i] < 0) {
      i++;
    } else if ((*I)[j] < 0 || i == j) {
      j++;
    } else {
      for (k = (*I)[i]; k < (*I)[j]; k++) {
        diag_offset = (*J)[k] - i;
        dlen = diag_len(rows, cols, diag_offset);
        if (dlen > max_diag_len)
          max_diag_len = dlen;
      }
      i++;
      j++;
    }
  }
  return max_diag_len;
}

/* IOFF output will be in increasing order
 *
 * returns the start index of non-negative diagonal
 * offsets in IOFF if any */
static int fill_ioff(int ndiags, int **IOFF, unsigned int **bit_arr_pos,
                     unsigned int **bit_arr_neg, int max_ndiags_pos,
                     int max_ndiags_neg) {
  int i, ioff_idx, word_width, bit_idx;
  int leading_zeros, diag_offset, trailing_zeros;
  int bit_arr_neg_len, bit_arr_pos_len;
  unsigned int word;
  SAFE_MALLOC(IOFF, ndiags);

  /* fill IOFF in increasing order of diagonal offsets
   *
   * can be done by traversing the bit array for negative diagonal
   * offsets first from higher 32-bit word msb then the positive offsets from
   * lower 32-bit word lsb. */
  word_width = sizeof(word) * BYTE_WIDTH; // in bits
  ioff_idx = 0;

  /* process negative offsets */
  /* +1 otherwise only covers indices in [0, n-2] when need
   * to cover n indices */
  bit_arr_neg_len = ceil(
        (float)max_ndiags_neg / (float)(sizeof(**bit_arr_neg) * BYTE_WIDTH)
    ) + 1;

  for (i = bit_arr_neg_len - 1; i >= 0; i--) {
    word = (*bit_arr_neg)[i];

    /* __builtin_clz results in UB if input is 0 */
    while (word != 0) {
      leading_zeros = __builtin_clz(word);
      bit_idx = word_width - leading_zeros - 1;

      diag_offset = (i * word_width) + bit_idx;
      assert(diag_offset > 0 && "'diag_offset' must be positive first!");
      (*IOFF)[ioff_idx] = -diag_offset; // must be negative

      ioff_idx++;
      /* clear the msb */
      // clear_bit(&word, bit_idx);
      word &= ~(1 << bit_idx);
    }
  }


  int start_pos_idx = ioff_idx;

  /* process positive offsets */
  /* +1 otherwise only covers indices in [0, n-2] when need
   * to cover n indices */
  bit_arr_pos_len = ceil(
        (float)max_ndiags_pos / (float)(sizeof(**bit_arr_pos) * BYTE_WIDTH)
    ) + 1;

  for (i = 0; i < bit_arr_pos_len; i++) {
    word = (*bit_arr_pos)[i];

    /* __builtin_ctz results in UB if input is 0 */
    while (word != 0) {
      trailing_zeros = __builtin_ctz(word);
      bit_idx = trailing_zeros;
      diag_offset = (i * word_width) + bit_idx;

      assert(diag_offset >= 0 && "'diag_offset' must be nonnegative!");
      (*IOFF)[ioff_idx] = diag_offset; // must be positive
      ioff_idx++;

      /* clear the lsb */
      // clear_bit(&word, bit_idx);
      word &= (word - 1);
    }
  }

  if (ioff_idx < ndiags) {
    fprintf(stderr, "%s:%s:%d: IOFF 'ioff_idx'(%d) < 'ndiags' (%d)\n",
            __FILE__, __func__, __LINE__, ioff_idx, ndiags);
    free(*IOFF);
    free(*bit_arr_pos);
    free(*bit_arr_neg);
    exit(EXIT_FAILURE);
  }

  return start_pos_idx;
}

static int bin_search(int **arr, int start, int end, int target) {
  int m, v;
  m = (start + end) / 2;
  while (start <= end) {
    v = (*arr)[m];
    if (v == target) {
      return m;
    } else if (target < v) {
      end = m - 1;
    } else {
      start = m + 1;
    }
    m = (start + end) / 2;
  }
  return NOT_FOUND;
}

static void fill_diag(double **A, int **I, int **J, double **DIAG, int **IOFF,
                      int i, int j, int N, int start_pos, int ndiags,
                      int **diags_next_idxs, unsigned int **bit_arr_pos,
                      unsigned int **bit_arr_neg) {
  int col, k_upbnd, row_idx, k;
  int diag_offset, diag_ioff_idx;

  col = 0;
  k = (*I)[i];
  k_upbnd = (*I)[j];
  while (col < N) {
    diag_offset = col - i;

    /* skip if current cell's diagonal doesn't have at least a nonzero */
    if ((diag_offset >= 0 && !test_bit(*bit_arr_pos, diag_offset))) {
      col++;
      continue;
    }
    if ((diag_offset < 0 && !test_bit(*bit_arr_neg, abs(diag_offset)))) {
      col++;
      continue;
    }

    /* find the diagonal's column index in IOFF (and in DIAG) */
    if (diag_offset == 0) {
      diag_ioff_idx = start_pos;
    } else if (diag_offset > 0) {
      /* binary search on the positive side */
      diag_ioff_idx = bin_search(IOFF, start_pos, ndiags - 1, diag_offset);
      if (diag_ioff_idx == NOT_FOUND) {
        fprintf(stderr, "%s:%s:%d: %d offset not found in 'IOFF'\n", __FILE__,
                __func__, __LINE__, diag_offset);
        exit(EXIT_FAILURE);
      }
    } else {
      /* binary search on the negative side */
      diag_ioff_idx = bin_search(IOFF, 0, start_pos - 1, diag_offset);
      if (diag_ioff_idx == NOT_FOUND) {
        fprintf(stderr, "%s:%s:%d: %d offset not found in 'IOFF'\n", __FILE__,
                __func__, __LINE__, diag_offset);
        exit(EXIT_FAILURE);
      }
    }

    /* remember that at this point we're at a diagonal
     * that contains at least a nonzero (i.e. potential "gap"). */

    /* current cell contains nonzero - no "gap" */
    /* k == -1 if i-th row has no nonzeros so don't
     * write a value from array A to array DIAG */
    if (k > -1 && k < k_upbnd && col == (*J)[k]) {
      /* get the next row index where to put
       * the nonzero for the diagonal */
      row_idx = (*diags_next_idxs)[diag_ioff_idx];
      (*DIAG)[row_idx * ndiags + diag_ioff_idx] = (*A)[k];

      /* go to the next col with nonzero */
      k++;
    }

    /* update next row index for the next
     * nonzero of the diagonal - or because
     * there was a "gap" */
    (*diags_next_idxs)[diag_ioff_idx]++;

    /* go to the next col */
    col++;
  } // while loop
}

/* preconds:
 * M = #rows + 1, due to being from CSR format
 * N = #cols
 * J array contains column indices of each row
 * in increasing order */
int csr2diag(double **A, int **I, int **J, int M, int N, double **DIAG,
             int **IOFF, int **max_diag_len) {
  /* verify that input pointers are allocated */
  POINTER_VERIFY(A);
  POINTER_VERIFY(I);
  POINTER_VERIFY(J);

  /* verify that output pointers are not yet allocated */
  VERIFY_NON_ALLOC(DIAG);
  VERIFY_NON_ALLOC(IOFF);

  if (M <= 0) {
    fprintf(stderr, "%s:%s:%d: 'M' must be > 0 [%s].\n", __FILE__, __func__,
            __LINE__, strerror(EINVAL));
    exit(EXIT_FAILURE);
  }
  if (N <= 0) {
    fprintf(stderr, "%s:%s:%d: 'N' must be > 0 [%s].\n", __FILE__, __func__,
            __LINE__, strerror(EINVAL));
    exit(EXIT_FAILURE);
  }

  /* by def. of diagonal format,
   * if there is at least one nonzero
   * on a diagonal then the whole diagonal
   * is included in the arrays by filling
   * positions that don't have nonzeros with
   * zeros.
   *
   * Obs: each diagonal can be uniquely
   * indentified by its offset from the main diagonal */

  /* from left2right:
   * each column index can start a diagonal
   * each row index can start a diagonal
   * => #cols + #rows - 1 is the max number of diagonals
   * where the -1 is because the main diagonal is counted
   * twice in cols and rows.
   *
   * the upper diagonals (those with positive offsets)
   * are #cols
   *
   * the lower diagonals (those with negative offsets)
   * are #rows - 1 (the -1 is for the main diagonal) */
  int max_ndiags_pos = N;
  int max_ndiags_neg = M - 2; // because M = #rows + 1

  unsigned int *bit_arr_pos, *bit_arr_neg;

  int ndiags = set_diagonals(max_ndiags_pos, max_ndiags_neg, &bit_arr_pos,
                             &bit_arr_neg, M, I, J);

  SAFE_MALLOC(max_diag_len, 1);
  **max_diag_len = longest_diag_len(M, N, I, J);

  CHECK_OVERFLOW((size_t)(**max_diag_len) > SIZE_MAX / ndiags,
                 "nmemb for 'DIAG'", exit(EXIT_FAILURE));

  /* Check that DIAG dimension
   * doesn't overflow w.r.t. size_t */
  CHECK_OVERFLOW((size_t)(**max_diag_len) * ndiags > SIZE_MAX / sizeof(**DIAG),
                 "'DIAG'", exit(EXIT_FAILURE));

  size_t diag_bytes = (**max_diag_len) * ndiags * sizeof(double);

  CHECK_OVERFLOW((size_t)ndiags > SIZE_MAX / sizeof(**IOFF), "'IOFF'",
                 exit(EXIT_FAILURE));

  size_t ioff_bytes = ndiags * sizeof(int);

  /* check that the memory allocated is not over the limit
   * defined in config.h */
  CHECK_VALIDITY_ALLOC(diag_bytes > MAX_ALLOC_BYTES ||
                           ioff_bytes > MAX_ALLOC_BYTES - diag_bytes,
                       exit(EXIT_FAILURE));

  *DIAG = calloc((**max_diag_len) * ndiags, sizeof(**DIAG));
  if (DIAG == NULL) {
    fprintf(stderr, "%s:%s:%d: Could not allocate memory.\n", __FILE__,
            __func__, __LINE__);
    exit(EXIT_FAILURE);
  }

  /* no diagonals so just return */
  if (ndiags <= 0) {
    return ndiags;
  }

  /* populate DIAG and IOFF */

  /* populate IOFF first, this will
   * decide the order of the diagonals */
  int start_pos;
  start_pos = fill_ioff(ndiags, IOFF, &bit_arr_pos, &bit_arr_neg,
                        max_ndiags_pos, max_ndiags_neg);

  /* populate DIAG using IOFF */
  int i, j, *diags_next_idxs;

  // tracks the row indices of the diagonals
  // in DIAG for the next element
  diags_next_idxs = calloc(ndiags, sizeof(*diags_next_idxs));
  if (diags_next_idxs == NULL) {
    fprintf(stderr, "%s:%s:%d: Could not allocate memory.\n", __FILE__,
            __func__, __LINE__);
    exit(EXIT_FAILURE);
  }

  i = 0;
  j = 1;
  while (j < M) {
    /* linear scan on the cols
     * of each row while taking into account "gaps"
     *
     * (assumes that column indices in J array of CSR
     * are in increasing order)
     *
     * check if the 'col' at i-th row
     * contains a nonzero or if there is "gap"
     *
     * where a "gap" is any cell
     * in the original matrix that contains
     * zero but belongs to a diagonal that
     * contains at least a nonzero
     * (important because we want to have '0'
     * in that gap and update the next element pointer
     * for its diagonal) */

    /* i-th row only has no nonzeros. */
    if ((*I)[i] < 0) {
      /* even though the rows only has zeros
       * it may contain "gap" that we need in DIAG */
      fill_diag(A, I, J, DIAG, IOFF, i, j, N, start_pos, ndiags,
                &diags_next_idxs, &bit_arr_pos, &bit_arr_neg);
      i++;
    } else if ((*I)[j] < 0 || i == j) {
      j++;
    } else {
      /* i-th row with nonzeros */

      fill_diag(A, I, J, DIAG, IOFF, i, j, N, start_pos, ndiags,
                &diags_next_idxs, &bit_arr_pos, &bit_arr_neg);
      i++;
      j++;
    }
  }

  /* clean up */
  free(bit_arr_pos);
  free(bit_arr_neg);
  free(diags_next_idxs);

  return ndiags;
}

#ifdef UNIT_TEST_CSR2DIAG

#include "../../mtx_mrkt_csr/include/api/csr_util.h"
#include "../../mtx_mrkt_csr/include/matrix_market/util.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "USAGE: ./a.out [filename]\n");
    return EXIT_FAILURE;
  }
  char *filename = argv[1];

  double *A = NULL;
  int *I = NULL, *J = NULL;
  int n_rows, i, nz;

  n_rows = mtx_mrkt2csr(filename, &A, &J, &I);
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

  double *DIAG = NULL;
  int *IOFF = NULL;
  int *max_diag_len, ndiags, row, diag, col_count;
  MM_DATA *mtx_data;
  mtx_data = read_mtx_data(filename);
  col_count = get_col_count(mtx_data);
  ndiags = csr2diag(&A, &I, &J, n_rows, col_count, &DIAG, &IOFF, &max_diag_len);

  /* print DIAG and IOFF by column */
  fprintf(stdout, "=== DIAG ===\n");
  for (row = 0; row < *max_diag_len; row++) {
    for (diag = 0; diag < ndiags; diag++)
      fprintf(stdout, " %lg | ", DIAG[row * ndiags + diag]);
    fprintf(stdout, "\n");
  }
  fprintf(stdout, "\n=== IOFF ===\n");
  for (diag = 0; diag < ndiags; diag++) {
    fprintf(stdout, " %d | ", IOFF[diag]);
  }
  fprintf(stdout, "\n\n");

  free(A);
  free(I);
  free(J);
  free(DIAG);
  free(IOFF);
  free(max_diag_len);
  free(mtx_data);

  return EXIT_SUCCESS;
}

#endif
