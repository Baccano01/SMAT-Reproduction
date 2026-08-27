/* parse_vector.c */

/* Contains API useful for parsing
 * vectors of type double. It can be
 * used for parsing vectors to be tested
 * for SpMv kernels.
 */

#include <stddef.h>

typedef struct {
  double *data;
  size_t size;
  size_t capacity;
} DoubleVec;

void dv_init(DoubleVec *v);
void dv_free(DoubleVec *v);
int dv_push(DoubleVec *v, double x);

/* PARAMS:
 * - *s is the pointer of string where to parse the double vector
 *   of the format '[x1, x2, ..., xn]'.
 * - *out is the pointer of the output. */
int parse_double_vector(const char *s, DoubleVec *out);
