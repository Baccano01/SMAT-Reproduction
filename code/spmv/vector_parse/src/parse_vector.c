/*
 * parse_vector.c
 *
 * Reads a vector of doubles from stdin in the form:
 *     [1.0, -2.5, 3.14159, 0, 42]
 *
 * Whitespace around brackets/commas/numbers is tolerated.
 * Grows a dynamic array as needed and returns it to the caller.
 *
 */

#include "../include/parse_vector.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dv_init(DoubleVec *v) {
  v->data = NULL;
  v->size = 0;
  v->capacity = 0;
}

void dv_free(DoubleVec *v) {
  free(v->data);
  v->data = NULL;
  v->size = v->capacity = 0;
}

int dv_push(DoubleVec *v, double x) {
  if (v->size == v->capacity) {
    size_t new_cap = v->capacity ? v->capacity * 2 : 16;
    double *tmp = realloc(v->data, new_cap * sizeof(double));
    if (!tmp)
      return -1; /* out of memory */
    v->data = tmp;
    v->capacity = new_cap;
  }
  v->data[v->size++] = x;
  return 0;
}

static int repeat_push(const char **s, char **endptr, DoubleVec *out,
                       double val) {
  const int base = 10;
  long occurences;
  (*s)++;
  while (isspace((unsigned char)**s))
    (*s)++;

  /* read the occurence count */
  errno = 0;
  occurences = strtol(*s, endptr, base);
  if (errno != 0) {
    fprintf(stderr, "%s:%s:%d: %s\n", __FILE__, __func__, __LINE__,
            strerror(errno));
    dv_free(out);
    return -3;
  }
  occurences--; // due to the first push to *out by the caller

  if (endptr == NULL || (**s != '\0' && **endptr == '\0')) {
    fprintf(stderr, "%s:%s:%d: expected ']'\n", __FILE__, __func__, __LINE__);
    dv_free(out);
    return -4;
  }

  if (*s == *endptr && occurences == 0) {
    fprintf(stderr, "%s:%s:%d: expected an integer\n", __FILE__, __func__,
            __LINE__);
    dv_free(out);
    return -5;
  }

  *s = *endptr;

  /* fill vector with 'occurences' times of 'val' */
  while (occurences--) {
    if (dv_push(out, val) != 0) {
      fprintf(stderr, "%s:%s:%d: out of memory\n", __FILE__, __func__,
              __LINE__);
      dv_free(out);
      return -2;
    }
  }
  return 0;
}

/*
 * Parse a string of the form "[ d1 , d2 , ... , dn ]" into out.
 * Returns 0 on success, -1 on a malformed/empty input, -2 on OOM.
 */
int parse_double_vector(const char *s, DoubleVec *out) {
  dv_init(out);

  while (isspace((unsigned char)*s))
    s++;
  if (*s != '[') {
    fprintf(stderr, "%s:%s:%d: expected '[' at start of input\n", __FILE__,
            __func__, __LINE__);
    return -1;
  }
  s++; /* consume '[' */

  while (isspace((unsigned char)*s))
    s++;

  /* Handle the empty-vector case: "[]" */
  if (*s == ']') {
    s++;
    return 0;
  }

  for (;;) {
    char *endptr = NULL;
    errno = 0;
    double val = strtod(s, &endptr);

    if (endptr == s) {
      fprintf(stderr, "%s:%s:%d: expected a number near \"%.20s\"\n", __FILE__,
              __func__, __LINE__, s);
      dv_free(out);
      return -1;
    }
    if (errno == ERANGE) {
      fprintf(stderr, "%s:%s:%d: value out of range near \"%.20s\"\n", __FILE__,
              __func__, __LINE__, s);
      dv_free(out);
      return -1;
    }

    if (dv_push(out, val) != 0) {
      fprintf(stderr, "%s:%s:%d: out of memory\n", __FILE__, __func__,
              __LINE__);
      dv_free(out);
      return -2;
    }

    s = endptr;
    while (isspace((unsigned char)*s))
      s++;

    if (*s == '@') {
      repeat_push(&s, &endptr, out, val);
    }

    if (*s == ',') {
      s++;
      while (isspace((unsigned char)*s))
        s++;
      continue;
    }
    if (*s == ']') {
      s++;
      break;
    }

    fprintf(stderr, "%s:%s:%d: expected ',' or ']' near \"%.20s\"\n", __FILE__,
            __func__, __LINE__, s);
    dv_free(out);
    return -1;
  }

  return 0;
}
