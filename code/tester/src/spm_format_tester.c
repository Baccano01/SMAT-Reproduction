/* Inputs:
 * - pointer to the element extractor function.
 * - vargs where to pass the data structure needed by the format being tested.
 * Output:
 * - 1 if correct
 * - 0 if incorrect
 * Sideffects:
 * - if there is an error it prints to the stderr a description of the error
 * - in case of success, it prints to stderr a success message */

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#ifndef _DIRENT_HAVE_D_TYPE
#include <sys/stat.h>
#endif

#include "../../csr2coo/include/csr2coo.h"
#include "../../csr2diag/include/csr2diag.h"
#include "../../csr2ell/include/csr2ell.h"
#include "../../mtx_mrkt_csr/include/api/csr_util.h"
#include "../../mtx_mrkt_csr/include/matrix_market/mmio.h"
#include "../../mtx_mrkt_csr/include/matrix_market/util.h"
#include <dirent.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define TEST_REPORT(format, f)                                                 \
  do {                                                                         \
    if ((f)) {                                                                 \
      fprintf(stdout, "+ %s: ok ✓\n", (format));                               \
    } else {                                                                   \
      fprintf(stdout, "+ %s: wrong x\n", (format));                            \
    }                                                                          \
  } while (0)

/// pointer to function for extracting elements in a sparse matrix format.
typedef int (*Search)(int, int, double **, int **, int **, double, int);

/* seaches for an entry via coordinates in a CSR format
 * returning 1 if found, else 0. */
int csr_search(int row, int col, double **A, int **I, int **J, double target,
               int tot_nz) {
  int start = (*I)[row];
  int end = row + 1;

  /* find the index of the next
   * row with nonzeros because
   * A[I[row], ..., I[end]] is the
   * row-th row's nonzeros. */
  while ((*I)[end] < 0)
    end++;

  /* finds the index of the nonzero
   * we want by comparing its col index. */
  for (int loc = start; loc < (*I)[end]; loc++) {
    if ((*J)[loc] == col)
      return (*A)[loc] == target;
  }
  /* indicates that we didn't found the target */
  return 0;
}

/* seaches for an entry via coordinates in a COO format
 * returning 1 if found, else 0. */
int coo_search(int row, int col, double **A, int **I, int **J, double target,
               int tot_nz) {
  /* WARNING: This is inefficient, but this is not the target
   * of the project objective.
   * Can be optimized with binary search tree. */
  int i;
  for (i = 0; i < tot_nz; i++) {
    if (row == (*I)[i] && col == (*J)[i] && target == (*A)[i])
      return 1;
  }
  return 0;
}

/* binary_search in arr[start ... end] */
int binary_search(int **arr, int start, int end, int target) {
  int m;
  double v;
  m = (start + end) / 2;
  while (start <= end) {
    v = (*arr)[m];
    if (target == v)
      return m;
    else if (target < v)
      end = m - 1;
    else
      start = m + 1;

    m = (start + end) / 2;
  }

  // target not found
  return -1;
}

/* precond:
 * - COEF and JCOEF are single arrays representing a 2D matrices.
 * - COEF and JCOEF nonzeros must be in the same order as they are
 *   found in the original sparse matrix (i.e. values in JCOEF
 *   in each row must be in increasing order). */
int ell_search(int row, int col, double **COEF, int **JCOEF, double target,
               int row_len) {
  // binary search starting from JCOEF[row * row_len] to
  // JCOEF[(row + 1) * row_len - (k + 1)], where k indicates the zeros
  // in the row 'row', to find col
  int start = row * row_len;
  int end = (row + 1) * row_len - 1;

  // go towards the first non sentinel column index
  while (start < end && JCOEF_SENTINEL == (*JCOEF)[end])
    end--;
  int idx = binary_search(JCOEF, start, end, col);

  // col not found
  if (idx == -1)
    return 0;

  // if col is found at index i then check COEF[i] == target
  return target == (*COEF)[idx];
}

int test_ell(char *mtx_mrkt_path, double **COEF, int **JCOEF, int row_len) {
  int ret_code, M, N, nz, test_res;
  MM_typecode matcode;
  FILE *f;
  test_res = 1; // success

  /* open the matrix market file with read */
  if (mtx_mrkt_path != NULL) {
    errno = 0;
    if ((f = fopen(mtx_mrkt_path, "r")) == NULL) {
      fprintf(stderr, "%s:%s:%d: Could not open file [%s] - %s\n", __FILE__,
              __func__, __LINE__, mtx_mrkt_path, strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  if ((ret_code = mm_read_banner(f, &matcode)) != 0) {
    fprintf(stderr,
            "%s:%s:%d: Could not process Matrix Market banner [%s] - %s.\n",
            __FILE__, __func__, __LINE__, mtx_mrkt_path, mm_strerror(ret_code));
    exit(EXIT_FAILURE);
  }

  /* find out size of sparse matrix .... */
  if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &nz)) != 0) {
    fprintf(stderr, "%s:%s:%d: Could not process matrix size [%s] - %s",
            __FILE__, __func__, __LINE__, mtx_mrkt_path, mm_strerror(ret_code));
    exit(EXIT_FAILURE);
  }

  int row, col, found;
  double val;
  for (int i = 0; i < nz; i++) {
    fscanf(f, "%d %d %lg\n", &row, &col, &val);
    /* Matrix Market is 1-indexed so
     * subtract 1 from row and col */
    row--;
    col--;
    found = ell_search(row, col, COEF, JCOEF, val, row_len);
    if (!found) {
      fprintf(stderr, "%lg not found in ELL arrays at y=%d, x=%d!\n", val, row,
              col);
      test_res = 0;
      break;
    }

    // check (skew) symmetric counter part.
    if ((mm_is_symmetric(matcode) || mm_is_skew(matcode)) && row != col) {
      found = ell_search(col, row, COEF, JCOEF, val, row_len);
      if (!found) {
        fprintf(stderr, "%lg not found in ELL arrays at y=%d, x=%d!\n", val,
                col, row);
        test_res = 0;
        break;
      }
    }
  }
  if (f != stdin)
    fclose(f);

  return test_res;
}

/* nz is the number of nonzeros in the Matrix Market file.
 * tot_nz is the actual number of nonzeros considering the symmetry. */
int run_search(FILE *f, Search searcher, int nz, int tot_nz,
               MM_typecode matcode, double **A, int **J, int **I,
               char *format) {
  int row, col, found;
  double val;
  for (int i = 0; i < nz; i++) {
    fscanf(f, "%d %d %lg\n", &row, &col, &val);
    /* Matrix Market is 1-indexed so
     * subtract 1 from row and col */
    row--;
    col--;
    found = searcher(row, col, A, I, J, val, tot_nz);
    if (!found) {
      fprintf(stderr, "%lg not found in %s arrays at y=%d, x=%d!\n", val,
              format, row, col);
      return 0;
    }

    /* Check the (skew) symmetric counterpart */
    if ((mm_is_symmetric(matcode) || mm_is_skew(matcode)) && row != col) {
      found = searcher(col, row, A, I, J, val, tot_nz);
      if (!found) {
        fprintf(stderr, "%lg not found in %s arrays at y=%d, x=%d!\n", val,
                format, col, row);
        return 0;
      }
    }
  }
  return 1;
}

/* tests whether arrays of a sparse matrix format
 * contains all nonzeros of the orig Matrix Market matrix
 * at correct positions.
 * NOTE: the matrices considered are (real/double). */
int test_csr_o_csr(char *mtx_mrkt_path, Search searcher, int argc, ...) {
  va_list valist;
  int ret_code, M, N, nz, test_res;
  MM_typecode matcode;
  FILE *f;
  test_res = 1; // success

  /* open the matrix market file with read */
  if (mtx_mrkt_path != NULL) {
    errno = 0;
    if ((f = fopen(mtx_mrkt_path, "r")) == NULL) {
      fprintf(stderr, "%s:%s:%d: Could not open file [%s] - %s\n", __FILE__,
              __func__, __LINE__, mtx_mrkt_path, strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  if ((ret_code = mm_read_banner(f, &matcode)) != 0) {
    fprintf(stderr,
            "%s:%s:%d: Could not process Matrix Market banner [%s] - %s.\n",
            __FILE__, __func__, __LINE__, mtx_mrkt_path, mm_strerror(ret_code));
    exit(EXIT_FAILURE);
  }

  /* find out size of sparse matrix .... */
  if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &nz)) != 0) {
    fprintf(stderr, "%s:%s:%d: Could not process matrix size [%s] - %s",
            __FILE__, __func__, __LINE__, mtx_mrkt_path, mm_strerror(ret_code));
    exit(EXIT_FAILURE);
  }

  /* initialize valist for number of arguments */
  va_start(valist, argc);

  if (searcher == csr_search) {
    double **A;
    int **I, **J;
    A = va_arg(valist, double **);
    J = va_arg(valist, int **);
    I = va_arg(valist, int **);
    if (!run_search(f, searcher, -1, -1, matcode, A, J, I, "CSR")) {
      test_res = 0;
    }
  } else if (searcher == coo_search) {
    double **A;
    int **I, **J, tot_nz;
    A = va_arg(valist, double **);
    J = va_arg(valist, int **);
    I = va_arg(valist, int **);
    tot_nz = va_arg(valist, int);
    if (!run_search(f, searcher, nz, tot_nz, matcode, A, J, I, "COO")) {
      test_res = 0;
    }
  }

  /* clean memory reserved for valist */
  va_end(valist);
  if (f != stdin)
    fclose(f);

  return test_res;
}

/* precond:
 * - DIAG's columns correspond to the diagonals with nonzeros
 * - IOFF's columns correspond to the diagonals with nonzeros
 * - IOFF is ordered increasingly
 * - DIAG is a 1D array that represents a 2D matrix */
int diag_search(int row, int col, double **DIAG, int **IOFF, double target,
                int ndiag, int max_diag_len) {
  /* Since diagonals with nonzeros are aligned (i.e. each i-th elements of the
   * diagonals are in the same row) in DIAG, then row index for each entry is
   * invariant. While the column index is the same as the index of its diagonal
   * offset in IOFF. In fact, the column index of an upper diagonal element is
   * affected by its diagonal offset. Upper Diagonal (including main diagonal)
   * - DIAG[i][j] = a[i][i + IOFF[j]] = a[i][i + col - i] = a[i][col]
   *
   * Since lower diagonals are shifted by some rows downwards (so cannot use its
   * original row index) and diagonals are aligned in DIAG, its row index in
   * DIAG is its original col index. While the column index is the same as the
   * index of its diagonal offset in IOFF. In fact, the row index of a lower
   * diagonal element is affected by its diagonal offset. Lower Diagonal
   * - DIAG[i][j] = a[i - IOFF[j]][i] = a[i - col + i][i] = a[2 * i - col][i]
   *
   * Obs: Symmetry w.r.t. main diagonal still holds in DIAG.
   * */
  int x, diag_offset, diag_row_idx;
  diag_offset = col - row;
  x = binary_search(IOFF, 0, ndiag - 1, diag_offset);

  // not found
  if (x == -1)
    return 0;

  /* DIAG row index changes depending if diagonal
   * is lower or upper, 'col' if lower else 'row' */
  diag_row_idx = diag_offset < 0 ? col : row;
  if (diag_row_idx * ndiag + x > ndiag * max_diag_len)
    return 0;

  return (*DIAG)[diag_row_idx * ndiag + x] == target;
}

int test_diag(char *mtx_mrkt_path, double **DIAG, int **IOFF, int ndiag,
              int max_diag_len) {
  FILE *f;
  MM_typecode matcode;
  int M, N, nz, ret_code;
  /* open the matrix market file with read */
  if (mtx_mrkt_path != NULL) {
    errno = 0;
    if ((f = fopen(mtx_mrkt_path, "r")) == NULL) {
      fprintf(stderr, "%s:%s:%d: Could not open file [%s] - %s.\n", __FILE__,
              __func__, __LINE__, mtx_mrkt_path, strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  if ((ret_code = mm_read_banner(f, &matcode)) != 0) {
    fprintf(stderr,
            "%s:%s:%d: Could not process Matrix Market banner [%s] - %s.\n",
            __FILE__, __func__, __LINE__, mtx_mrkt_path, mm_strerror(ret_code));
    exit(EXIT_FAILURE);
  }

  /* find out size of sparse matrix .... */
  if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &nz)) != 0) {
    fprintf(stderr, "%s:%s:%d: Could not process matrix size [%s] - %s.\n",
            __FILE__, __func__, __LINE__, mtx_mrkt_path, mm_strerror(ret_code));
    exit(EXIT_FAILURE);
  }

  /* search for any missing nonzeros */
  int row, col, found;
  double val;
  for (int i = 0; i < nz; i++) {
    fscanf(f, "%d %d %lg\n", &row, &col, &val);
    /* Matrix Market is 1-indexed */
    row--;
    col--;
    found = diag_search(row, col, DIAG, IOFF, val, ndiag, max_diag_len);
    if (!found) {
      fprintf(stderr, "%lg not found in DIAG arrays at y=%d, x=%d!\n", val, row,
              col);
      return 0; // fail
    }

    /* Check the (skew) symmetric counterpart */
    if ((mm_is_symmetric(matcode) || mm_is_skew(matcode)) && row != col) {
      found = diag_search(col, row, DIAG, IOFF, val, ndiag, max_diag_len);
      if (!found) {
        fprintf(stderr, "%lg not found in DIAG arrays at y=%d, x=%d!\n", val,
                col, row);
        return 0; // fail
      }
    }
  }

  return 1; // success
}

// ==============
// NON TEST FUNCS
// ==============
int compute_nz_csr(int **I, int M) {
  int i;
  /* finds the first non-negative value in I
   * which will need to calculate the number
   * of non-zeros nz */
  for (i = 0; i < M; i++) {
    if ((*I)[i] > -1)
      break;
  }
  return (*I)[M - 1] - (*I)[i];
}

// Source - https://stackoverflow.com/a/24976006
// Posted by G--, modified by community. See post 'Timeline' for change history
// Retrieved 2026-08-06, License - CC BY-SA 3.0

const char *get_filename_ext(const char *filename) {
  const char *dot = strrchr(filename, '.');
  if (!dot || dot == filename)
    return "";
  return dot + 1;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "USAGE: ./a.out [directory] [ext]\n");
    exit(EXIT_FAILURE);
  }

  int is_file;
  DIR *dir;
  struct dirent *entry;

  errno = 0;
  dir = opendir(argv[1]);
  if (dir == NULL) {
    fprintf(stderr, "%s:%s:%d: Failed to open directory [%s] - %s\n", __FILE__,
            __func__, __LINE__, argv[1], strerror(errno));
    exit(EXIT_FAILURE);
  }

  errno = 0;
  is_file = 0;
  while ((entry = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
    is_file = DT_REG == entry->d_type;
#else
    struct stat buffer;
    int status;
    is_file = stat(entry->d_name, &buffer);
    if (status == -1) {
      fprintf(stderr, "%s:%s:%d: Failed to stat item - %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    if (buffer.st_mode & S_IFREG) {
      is_file = 1;
    }
#endif
    if (is_file) {
      int IS_SUPPORTED_EXT = !strcmp(get_filename_ext(entry->d_name), argv[2]);
      if (IS_SUPPORTED_EXT) {
        char path[PATH_MAX + 1];
        char buff[strlen(argv[1]) + strlen(entry->d_name) + 1];
        snprintf(buff, strlen(argv[1]) + 1, "%s%s", argv[1], "/");
        strcat(buff, entry->d_name);

        errno = 0;
        if (realpath(buff, path) == NULL && errno) {
          fprintf(stderr, "%s:%s:%d: Error getting real path [%s] - %s\n",
                  __FILE__, __func__, __LINE__, buff, strerror(errno));
          exit(EXIT_FAILURE);
        }

        /* CSR setup */
        double *A = NULL;
        int *I = NULL, *J = NULL;
        // M = #rows + 1, it's the length of I in CSR
        int M = mtx_mrkt2csr(path, &A, &J, &I);
        int nz = compute_nz_csr(&I, M);

#if 1
        /* COO setup */
        double *CA = NULL;
        int *CI = NULL, *CJ = NULL;

        csr2coo(&A, &J, &I, M, &CA, &CJ, &CI);
        // test_csr_o_csr(path, csr_search, 3, &A, &J, &I);
        // test_csr_o_csr(path, coo_search, 4, &CA, &CJ, &CI, nz);
#endif

#if 1
        /* ELLPACK-ITPACK setup */
        double *COEF = NULL;
        int *JCOEF = NULL, row_len;
        row_len = csr2ell(&A, &I, &J, M, &COEF, &JCOEF);
        // test_ell(path, &COEF, &JCOEF, row_len);
#endif

        /* DIAG setup */
        double *DIAG = NULL;
        int *IOFF = NULL, *max_diag_len, N;
        MM_DATA *mm_data;
        mm_data = read_mtx_data(path);
        N = get_col_count(mm_data);

        int ndiags = csr2diag(&A, &I, &J, M, N, &DIAG, &IOFF, &max_diag_len);
        // test_diag(path, &DIAG, &IOFF, ndiags, *max_diag_len);
#if 1
        fprintf(stdout, "Input: [%s]\n", buff);
        TEST_REPORT("CSR", test_csr_o_csr(path, csr_search, 3, &A, &J, &I));

        TEST_REPORT("COO",
                    test_csr_o_csr(path, coo_search, 4, &CA, &CJ, &CI, nz));

        TEST_REPORT("ELL", test_ell(path, &COEF, &JCOEF, row_len));

        TEST_REPORT("DIA",
                    test_diag(path, &DIAG, &IOFF, ndiags, *max_diag_len));
#endif

        free(A);
        free(I);
        free(J);
#if 1
        free(CA);
        free(CI);
        free(CJ);
#endif

#if 1
        free(COEF);
        free(JCOEF);
#endif
        free(DIAG);
        free(IOFF);
        free(max_diag_len);
        free(mm_data);
      }
    }
    errno = 0;
  }
  if (entry == NULL && errno) {
    fprintf(stderr, "%s:%s:%d: Error while navigating dir stream - %s\n",
            __FILE__, __func__, __LINE__, strerror(errno));
    exit(EXIT_FAILURE);
  }

  /* clean up */
  closedir(dir);
  return EXIT_SUCCESS;
}
