/// @file util.h
/**
 * Contains functions and structs useful
 * for simplifying the read of Matrix Market
 * matrices.
 *
 * Copyright 2026 Jofeth Abello
 */

#ifndef UTIL_H
#define UTIL_H

#include "./mmio.h"

// forward declaration of an opaque pointer for encapsulation
// used by those that include util.h
typedef struct MM_DATA MM_DATA;

/* mm_data.c */
/** Reads metadata of a Matrix Market matrix.
 * @param[in] mtx_mrkt_path filename of the Matrix Market file. */
MM_DATA *read_mtx_data(char *mtx_mrkt_path);

/** Returns the nonzeros count */
int get_nnz(MM_DATA *mtx_data);

/** Returns the row count */
int get_row_count(MM_DATA *mtx_data);

/** Returns the column count */
int get_col_count(MM_DATA *mtx_data);

/** Returns a code that encodes the type of matrix */
MM_typecode *get_mm_typcode(MM_DATA *mtx_data);

#endif
