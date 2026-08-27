/// @file mm_data.c
/**
 * Contains the implementation of functions
 * useful for simplifying the read of Matrix
 * Market matrices.
 *
 * Copyright 2026 Jofeth Abello
 */

#include "../include/matrix_market/mmio.h"
#include "../include/matrix_market/util.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NULL_CHECK(ptr)                                                        \
  do {                                                                         \
    if ((ptr) == NULL) {                                                       \
      fprintf(stderr, "%s:%s:%d: '%s' is NULL\n", __FILE__, __func__,          \
              __LINE__, #ptr);                                                 \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

struct MM_DATA {
  int nnz;
  int row_count;
  int col_count;
  MM_typecode matcode;
};

MM_DATA *read_mtx_data(char *mtx_mrkt_path) {
  NULL_CHECK(mtx_mrkt_path);

  FILE *f;
  int ret_code;
  MM_DATA *data;

  errno = 0;
  data = malloc(sizeof(*data));
  if (data == NULL) {
    fprintf(stderr, "%s:%s:%d: failed to allocate memory - %s\n", __FILE__,
            __func__, __LINE__, strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (mtx_mrkt_path != NULL) {
    errno = 0;
    if ((f = fopen(mtx_mrkt_path, "r")) == NULL) {
      fprintf(stderr, "%s:%s:%d: failed to open file - %s\n", __FILE__,
              __func__, __LINE__, strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  if ((ret_code = mm_read_banner(f, &data->matcode)) != 0) {
    fprintf(stderr, "%s:%s:%d: %s\n", __FILE__, __func__, __LINE__,
            mm_strerror(ret_code));
    fprintf(stderr, "Could not process Matrix Market banner.\n");
    exit(EXIT_FAILURE);
  }
  /* find out size of sparse matrix .... */
  ret_code =
      mm_read_mtx_crd_size(f, &data->row_count, &data->col_count, &data->nnz);
  if (ret_code != 0) {
    fprintf(stderr, "%s:%s:%d: %s\n", __FILE__, __func__, __LINE__,
            mm_strerror(ret_code));
    fprintf(stderr, "Could not process Matrix Market size.\n");
    exit(EXIT_FAILURE);
  }

  fclose(f);
  return data;
}

int get_nnz(MM_DATA *mtx_data) {
  NULL_CHECK(mtx_data);
  return mtx_data->nnz;
}

int get_row_count(MM_DATA *mtx_data) {
  NULL_CHECK(mtx_data);
  return mtx_data->row_count;
}

int get_col_count(MM_DATA *mtx_data) {
  NULL_CHECK(mtx_data);
  return mtx_data->col_count;
}

MM_typecode *get_mm_typcode(MM_DATA *mtx_data) {
  NULL_CHECK(mtx_data);
  return &mtx_data->matcode;
}
