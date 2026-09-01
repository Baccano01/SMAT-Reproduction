#include "../../../../../mtx_mrkt_csr/include/matrix_market/mmio.h"
#include <stdio.h>

// USAGE: ./a.out [filepath.mtx]
// Prints the filepath of Matrix Market file and column count.
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "USAGE: ./a.out [filepath.mtx]\n");
        return 1;
    }

    FILE *f;
    char *filepath;
    int ret_code, rowsCount, colsCount, nnz;

    filepath = argv[1];
    if ((f = fopen(filepath, "r")) == NULL) {
        fprintf(stderr, "%s:%s:%d failed to open [%s]\n",
                __FILE__, __func__, __LINE__, filepath);
        return 1;
    }

    if ((ret_code = mm_read_mtx_crd_size(f, &rowsCount, &colsCount, &nnz)) != 0) {
        fprintf(stderr, "%s:%s:%d Could not process Matrix Market size\n",
                __FILE__, __func__, __LINE__);
        return 1;
    }

    printf("%s,%d\n", filepath, colsCount);
    return 0;
}
