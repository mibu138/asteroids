#ifndef DEF_H
#define DEF_H

#include <stdio.h>

#ifndef VERBOSE
#define VERBOSE 1
#endif

#if VERBOSE > 0
#define V1_PRINT(msg, args...) printf(msg, ## args)
#else
#define V1_PRINT(msg, args...)
#endif

#endif /* end of include guard: DEF_H */
