#ifndef _ZIP_COMMON_H
#define _ZIP_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlib.h"
#if (ZLIB_VERNUM < 0x1270)
typedef uLongf z_crc_t;
#endif

#include "ioapi.h"

#ifdef STDC
#  include <stddef.h>
#  include <string.h>
#  include <stdlib.h>
#endif
#ifdef NO_ERRNO_H
    extern int errno;
#else
#   include <errno.h>
#endif

#define LOCALHEADERMAGIC    (0x04034b50)
#define DESCRIPTORHEADERMAGIC    (0x08074b50)
#define CENTRALHEADERMAGIC  (0x02014b50)
#define ENDHEADERMAGIC      (0x06054b50)
#define ZIP64ENDHEADERMAGIC      (0x06064b50)
#define ZIP64ENDLOCHEADERMAGIC   (0x07064b50)

#ifndef QUAZIP_LOCAL
#  define QUAZIP_LOCAL static
#endif
/* compile with -DQUAZIP_LOCAL if your debugger can't find static symbols */

#define LOCALHEADERMAGIC    (0x04034b50)
#define DESCRIPTORHEADERMAGIC    (0x08074b50)
#define CENTRALHEADERMAGIC  (0x02014b50)
#define ENDHEADERMAGIC      (0x06054b50)
#define ZIP64ENDHEADERMAGIC      (0x06064b50)
#define ZIP64ENDLOCHEADERMAGIC   (0x07064b50)

#define ZIP_OK                          (0)
#define ZIP_END_OF_LIST_OF_FILE         (-100)
#define ZIP_ERRNO                       (Z_ERRNO)
#define ZIP_EOF                         (0)
#define ZIP_PARAMERROR                  (-102)
#define ZIP_BADZIPFILE                  (-103)
#define ZIP_INTERNALERROR               (-104)
#define ZIP_CRCERROR                    (-105)

#ifndef BUFREADCOMMENT
#define BUFREADCOMMENT (0x400)
#endif

#ifndef ALLOC
# define ALLOC(size) (malloc(size))
#endif
#ifndef TRYFREE
# define TRYFREE(p) {if (p) free(p);}
#endif

#ifndef ZIP_BUFSIZE
#define ZIP_BUFSIZE (16384)
#endif

#ifndef UNZ_MAXFILENAMEINZIP
#define ZIP_MAXFILENAMEINZIP (256)
#endif

#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)

#ifndef _ZLIB_H
#include "zlib.h"
#endif

#ifndef _ZLIBIOAPI_H
#include "ioapi.h"
#endif

#ifdef HAVE_BZIP2
#include "bzlib.h"
#endif

#define Z_BZIP2ED 12

#ifdef __cplusplus
extern "C" {
#endif

static const char copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - http://www.winimage.com/zLibDll";
int getByte(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, int *pi);
int getShort(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong *pX);
int getLong(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong *pX);
int getLong64(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T *pX);

ZPOS64_T SearchCentralDir (const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream);
ZPOS64_T SearchCentralDir64(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream);

#ifdef __cplusplus
}
#endif
#endif// _ZIP_COMMON_H
