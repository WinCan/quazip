#include "common.h"

/****************************************************************************/

int getByte(const zlib_filefunc64_32_def* pzlib_filefunc_def,voidpf filestream,int* pi)
{
    unsigned char c;
    int err = (int)ZREAD64(*pzlib_filefunc_def,filestream,&c,1);
    if (err==1)
    {
        *pi = (int)c;
        return ZIP_OK;
    }
    else
    {
        if (ZERROR64(*pzlib_filefunc_def,filestream))
            return ZIP_ERRNO;
        else
            return ZIP_EOF;
    }
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/

int getShort (const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX)
{
    int i = 0;
    int err = getByte(pzlib_filefunc_def,filestream,&i);
    uLong x = (uLong)i;

    if (err==ZIP_OK)
        err = getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    if (err==ZIP_OK)
        *pX = x;
    else
        *pX = 0;
    return err;
}



int getLong (const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX)
{
    int i = 0;
    int err = getByte(pzlib_filefunc_def,filestream,&i);
    uLong x = (uLong)i;

    for(unsigned char shift = 8 ; shift < 32 ; shift+=8)
    {
        if (err==ZIP_OK)
            err = getByte(pzlib_filefunc_def,filestream,&i);
        x += ((uLong)i)<<shift;
    }

    if (err==ZIP_OK)
        *pX = x;
    else
        *pX = 0;
    return err;
}




int getLong64 (const zlib_filefunc64_32_def* pzlib_filefunc_def,
                           voidpf filestream,
                           ZPOS64_T *pX)
{
  int i = 0;
  int err = getByte(pzlib_filefunc_def,filestream,&i);
  ZPOS64_T x = (ZPOS64_T)i;

  for(unsigned char shift = 8 ; shift < 64 ; shift+=8)
  {
      if (err==ZIP_OK)
          err = getByte(pzlib_filefunc_def,filestream,&i);
      x |= ((ZPOS64_T)i)<<shift;
  }

  if (err==ZIP_OK)
    *pX = x;
  else
    *pX = 0;

  return err;
}

ZPOS64_T SearchCentralDir(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream)
{
  unsigned char* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;

  if (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    return 0;


  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  if (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (unsigned char*)ALLOC(BUFREADCOMMENT+4);
  if (buf==NULL)
    return 0;

  uBackRead = 4;
  while (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos ;
    int i;
    if (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    else
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    if (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      break;

    if (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      break;

    for (i=(int)uReadSize-3; (i--)>0;)
    {
      if (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        uPosFound = uReadPos+i;
        break;
      }
    }

    if (uPosFound!=0)
      break;
  }
  TRYFREE(buf);
  return uPosFound;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/

ZPOS64_T SearchCentralDir64(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream)
{
  unsigned char* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;
  uLong uL;
  ZPOS64_T relativeOffset;

  if (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    return 0;

  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  if (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (unsigned char*)ALLOC(BUFREADCOMMENT+4);
  if (buf==NULL)
    return 0;

  uBackRead = 4;
  while (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos;
    int i;
    if (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    else
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    if (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      break;

    if (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      break;

    for (i=(int)uReadSize-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locater */
      if (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        uPosFound = uReadPos+i;
        break;
      }
    }

      if (uPosFound!=0)
        break;
  }

  TRYFREE(buf);
  if (uPosFound == 0)
    return 0;

  /* Zip64 end of central directory locator */
  if (ZSEEK64(*pzlib_filefunc_def,filestream, uPosFound,ZLIB_FILEFUNC_SEEK_SET)!=0)
    return 0;

  /* the signature, already checked */
  if (getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    return 0;

  /* number of the disk with the start of the zip64 end of  central directory */
  if (getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    return 0;
  if (uL != 0)
    return 0;

  /* relative offset of the zip64 end of central directory record */
  if (getLong64(pzlib_filefunc_def,filestream,&relativeOffset)!=ZIP_OK)
    return 0;

  /* total number of disks */
  if (getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    return 0;
  if (uL != 1)
    return 0;

  /* Goto Zip64 end of central directory record */
  if (ZSEEK64(*pzlib_filefunc_def,filestream, relativeOffset,ZLIB_FILEFUNC_SEEK_SET)!=0)
    return 0;

  /* the signature */
  if (getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    return 0;

  if (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    return 0;

  return relativeOffset;
}
