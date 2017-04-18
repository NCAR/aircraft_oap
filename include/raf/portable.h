
#ifndef PORTABLE_H
#define PORTABLE_H

#include <sys/types.h>
#include <netinet/in.h>


#if defined(__LITTLE_ENDIAN) || defined(_LITTLE_ENDIAN) || defined(LITTLE_ENDIAN)

#ifdef __cplusplus
extern "C" {
#endif
                                                                                          
float   ntohf(float);
                                                                                          
#ifdef __cplusplus
}
#endif


#else

#define ntohf(x)        (x)

#endif

#define htonf        ntohf

#endif
