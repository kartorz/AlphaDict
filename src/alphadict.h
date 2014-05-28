#ifndef _ALPHADICT_H_
#define _ALPHADICT_H_
# ifdef WIN32
#include <Windows.h>
#include "win32/config.h"
# endif
#include <stdio.h>
#include <stdlib.h>

#include "type.h"
#ifdef _LINUX
#include "config.h"
#endif
#define AL_VERSION  "1.0"

#define AL_ASSERT(cond, msg)  \
do { \
if (!(cond)) {\
	fprintf(stderr, "%s-(%s:%d)\n", msg, __FILE__,__LINE__);\
	exit(EXIT_FAILURE); }\
}while(0)

/* i in [0 .. c-1] */
#define LOOP(size)  for (int i=0; i<size; i++)

#define ITER_ADVANCE(iter, n)    \
do { \
for (int i=0; i<n; i++) \
    iter++; \
}while(0)

#define ELEMENT_ADVANCE(e, n)    \
do { \
for (int i=0; i<n; i++) \
    e = e->NextSiblingElement(); \
}while(0)

#endif
