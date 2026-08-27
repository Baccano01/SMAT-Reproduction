#ifndef PARAM_VERIFY
#define PARAM_VERIFY

#include <stdio.h>

#define SAFE_MALLOC(ptr, n)                                                    \
  do {                                                                         \
    *(ptr) = malloc((n) * sizeof(**(ptr)));                                    \
    if (*(ptr) == NULL) {                                                      \
      fprintf(stderr, "%s:%s:%d failed to allocate '%s'\n", __FILE__,          \
              __func__, __LINE__, #ptr);                                       \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

#define POINTER_VERIFY(ptr)                                                    \
  do {                                                                         \
    if ((ptr) == NULL) {                                                       \
      fprintf(stderr, "%s:%s:%d pointer '%s' is NULL!\n", __FILE__, __func__,  \
              __LINE__, #ptr);                                                 \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

#define VERIFY_NON_ALLOC(ptr)                                                  \
  do {                                                                         \
    if (*(ptr) != NULL) {                                                      \
      fprintf(stderr, "%s:%s:%d: '*%s' must be NULL.\n", __FILE__,             \
              __FUNCTION__, __LINE__, #ptr);                                   \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

#define CHECK_OVERFLOW(ineq, desc, handler)                                    \
  do {                                                                         \
    if ((ineq)) {                                                              \
      fprintf(stderr, "%s:%s:%d: %s [%s].\n", __FILE__, __func__, __LINE__,    \
              (desc), strerror(EOVERFLOW));                                    \
      handler;                                                                 \
    }                                                                          \
  } while (0)
#endif // PARAM_VERIFY
