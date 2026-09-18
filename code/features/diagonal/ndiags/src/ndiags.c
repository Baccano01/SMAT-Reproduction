/**
 * This contains function(s) for calculating the number of diagonals
 * of a sparse matrix. A diagonal is considered if it contains at least
 * a non-zero.
 *
 * Copyright 2026 Jofeth Abello
 */

#include "../../../../mtx_mrkt_csr/include/api/csr_util.h"
#include "../../../../mtx_mrkt_csr/include/matrix_market/util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>

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
  bit_arr_len =
      ceil((float)max_ndiags_pos / (float)(sizeof(**bit_arr_pos) * BYTE_WIDTH)) + 1;

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
  bit_arr_len =
      ceil((float)max_ndiags_neg / (float)(sizeof(**bit_arr_neg) * BYTE_WIDTH)) + 1;

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


double ER_DIAG(double nnz, double ndiags, double rows_count) {
    if (ndiags <= 0) {
        fprintf(stderr, "%s:%s:%d: Could not allocate memory.\n", __FILE__,
                __func__, __LINE__);
        exit(EXIT_FAILURE);
    }
    if (rows_count <= 0) {
        fprintf(stderr, "%s:%s:%d: Could not allocate memory.\n", __FILE__,
                __func__, __LINE__);
        exit(EXIT_FAILURE);
    }
    return nnz / (ndiags * rows_count);
}


/* prints the number of diagonals with at least a nonzero */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "USAGE: ./a.out [filename]\n");
        return EXIT_FAILURE;
    }

    char *filename = argv[1];

    double *A = NULL;
    int *J = NULL, *I = NULL;
    int M, nnz, i;

    M = mtx_mrkt2csr(filename, &A, &J, &I);

    int cols_count;
    long ndiags;
    MM_DATA *mtx_data;
    mtx_data = read_mtx_data(filename);
    cols_count = get_col_count(mtx_data);
    nnz = get_nnz(mtx_data);

    int max_ndiags_pos = cols_count;
    int max_ndiags_neg = M - 2;

    unsigned int *bit_arr_pos, *bit_arr_neg;

    ndiags = set_diagonals(max_ndiags_pos, max_ndiags_neg, &bit_arr_pos,
                           &bit_arr_neg, M, &I, &J);

    double er_diag;
    er_diag = ER_DIAG(nnz, ndiags, M - 1);

    printf("%ld,%lg", ndiags, er_diag);

    free(bit_arr_neg);
    free(bit_arr_pos);
    free(A);
    free(J);
    free(I);
    free(mtx_data);

    return EXIT_SUCCESS;
}
