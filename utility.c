/******************************************************************************
 This file is part of AmiNonogram.
 Copyright (C) 2022 Ibrahim Alper Sönmez
******************************************************************************/

///includes
//standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

//Amiga headers
#include <exec/exec.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>

//Amiga protos
#include <clib/alib_protos.h>
#include <proto/dos.h>
#include <proto/exec.h>

//SDI headers
#include <SDI_compiler.h>
#include <SDI_hook.h>
#include <SDI_interrupt.h>
#include <SDI_lib.h>
#include <SDI_misc.h>
#include <SDI_stdarg.h>
///
///globals
extern APTR memoryPool;
///

//Utility Functions
///Exists(fileName)
/********************************************
 * Checks if a filename is existent on disk *
 ********************************************/
BOOL Exists(STRPTR filename)
{
  BPTR lock;

  if (filename)
  {
    if((lock = Lock(filename, SHARED_LOCK)))
    {
      UnLock(lock);
      return TRUE;
    }
    if(IoErr() == ERROR_OBJECT_IN_USE)
      return TRUE;
  }

return FALSE;
}
///
///readString(fh)
/******************************************************************************
 * Reads a null terminated string from the given file handle (reading from    *
 * the current file location) and returns it as a newly allocated string.     *
 ******************************************************************************/
STRPTR readString(BPTR fh) {
  UBYTE buffer[256];
  UBYTE ch;
  STRPTR string = NULL;
  ULONG i = 0;

  while (TRUE) {
    Read(fh, &ch, 1);
    buffer[i++] = ch;
    if (ch == 0) break;
  }

  string = AllocPooled(memoryPool, i);
  if (string) {
    strcpy(string, buffer);
  }

  return string;
}
///
///makeString{num}(...)
/******************************************************************************
 * Creates a string allocated in program's memoryPool from all the strings    *
 * passed.                                                                    *
 ******************************************************************************/
STRPTR makeString(STRPTR str) {
  STRPTR result = AllocPooled(memoryPool, strlen(str) + 1);
  if (result) strcpy(result, str);

  return result;
}

STRPTR makeString2(STRPTR str1, STRPTR str2) {
  ULONG len = strlen(str1) + strlen(str2) + 1;
  STRPTR result = AllocPooled(memoryPool, len);

  if (result) {
    strcpy(result, str1);
    strcat(result, str2);
  }

  return result;
}
/*
STRPTR makeString3(STRPTR str1, STRPTR str2, STRPTR str3) {
  ULONG len = strlen(str1) + strlen(str2) + strlen(str2) + 1;
  STRPTR result = AllocPooled(memoryPool, len);

  if (result) {
    strcpy(result, str1);
    strcat(result, str2);
    strcat(result, str3);
  }

  return result;
}
*/
///
///makePath(directory, filename, extension)
/******************************************************************************
 * Adds a file name to a directory name in a newly allocated string from      *
 * program's memoryPool.                                                      *
 ******************************************************************************/
STRPTR makePath(STRPTR dir, STRPTR file, STRPTR extension) {
  STRPTR result = NULL;
  ULONG len = 0;

  if (file) {
    if (dir) {
      if (extension) {
        len = strlen(dir) + strlen(file) + strlen(extension) + 2;
        result = AllocPooled(memoryPool, len);
        if (result) {
          strcpy(result, dir);
          AddPart(result, file, len);
          strcat(result, extension);
        }
      }
      else {
        len = strlen(dir) + strlen(file) + 2;
        result = AllocPooled(memoryPool, len);
        if (result) {
          strcpy(result, dir);
          AddPart(result, file, len);
        }
      }
    }
    else {
      if (extension) {
        len = strlen(file) + strlen(extension) + 1;
        result = AllocPooled(memoryPool, len);
        if (result) {
          strcpy(result, file);
          strcat(result, extension);
        }
      }
      else {
        len = strlen(file) + 1;
        result = AllocPooled(memoryPool, len);
        if (result) {
          strcpy(result, file);
        }
      }
    }
  }
  return result;
}
///
///stripExtension(filename)
/******************************************************************************
 * Returns a new string with all the extension characters removed from the    *
 * given filename. Will return NULL for a filename like ".ext".               *
 ******************************************************************************/
STRPTR stripExtension(STRPTR filename) {
  STRPTR result = NULL;
  UBYTE *cursor = filename;
  ULONG len = 0;

  while (*cursor) {
    if (*cursor == '.') break;
    len++;
    cursor++;
  }

  if (len) {
    result = AllocPooled(memoryPool, len + 1);
    strncpy(result, filename, len);
    result[len] = 0;
  }

  return result;
}
///
///freeString(str)
/******************************************************************************
 * Frees a string created with makeString() or makePath()                     *
 ******************************************************************************/
VOID freeString(STRPTR str) {
  if (str) FreePooled(memoryPool, str, strlen(str) + 1);
}
///
