/**
 * Prints nnz, rows count, and columns count of a Matrix Market matrix.
 *
 * Copyright 2026 Jofeth Abello
 */

#include "../../../mtx_mrkt_csr/include/matrix_market/util.h"
#include <stdlib.h>
#include <stdio.h>

/* prints the number of diagonals with at least a nonzero */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "USAGE: ./a.out [filename]\n");
        return EXIT_FAILURE;
    }

    char *filename = argv[1];

    long cols_count, rows_count, nnz;
    MM_DATA *mtx_data;

    mtx_data = read_mtx_data(filename);
    cols_count = get_col_count(mtx_data);
    rows_count = get_row_count(mtx_data);
    nnz = get_nnz(mtx_data);

    printf("%ld,%ld,%ld", rows_count, cols_count, nnz);

    free(mtx_data);

    return EXIT_SUCCESS;
}
