#ifndef _TYPE_H_
#define _TYPE_H_

typedef unsigned  char   u8;
typedef unsigned  short u16;
typedef unsigned  int   u32;
typedef unsigned  int   u4char_t;

typedef unsigned int address_t;

enum APP_ERRCODE {
    ERR_MKHOME = -5,
    ERR_CPCFG,
    ERR_LDCFG,
};

#ifdef WIN32
typedef unsigned int pthread_t;
typedef void* HANDLE;

#define getopt  pgetopt
#define optarg  poptarg
#define optind  poptind
#define opterr  popterr
#define ftello  ftell
#define getcwd  _getcwd

#define S_ISDIR(x) x&S_IFDIR
#endif

#endif
