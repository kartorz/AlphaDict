#ifndef POSIXTOISO
#define  POSIXTOISO

#include <io.h>
#include <stdio.h>
#include <process.h>

#define unlink(a)  _unlink(a)
#define getpid() _getpid()

#endif
