/// @file config.h
/** Configuration header to define
 * limits of the application.
 *
 * Copyright 2026 Jofeth Abello
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

/// 250 GiB memory allocation limit
#define MAX_ALLOC_BYTES (250ULL * 1024 * 1024 * 1024)

#define CHECK_VALIDITY_ALLOC(ineq, handler)                                    \
  do {                                                                         \
    if ((ineq)) {                                                              \
      fprintf(stderr, "%s:%s:%d: bytes over the limit in 'config.h'\n",        \
              __FILE__, __func__, __LINE__);                                   \
      handler;                                                                 \
    }                                                                          \
  } while (0)

#endif // CONFIG_H
