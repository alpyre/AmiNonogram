/******************************************************************************
 This file is part of AmiNonogram.
 Copyright (C) 2022 Ibrahim Alper Sönmez

 AmiNonogram is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 AmiNonogram is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with AmiNonogram. If not, see <https://www.gnu.org/licenses/>.
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
#if defined(__amigaos4__)
  #include <dos/obsolete.h>
#endif

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

#include "utility.h"
#include "pack.h"
///
///globals
extern APTR memoryPool;
///

Pack* newPack(STRPTR name, BPTR fh) {
  Pack* pack = AllocPooled(memoryPool, sizeof(Pack));

  if (pack) {
    UBYTE buf[4] = {0};
    UBYTE compatibilityVersion;
    UBYTE fileVersion;
    UBYTE fileRelease;
    UBYTE numSizes;
    UBYTE *sizes;
    UWORD *numPuzzles;
    UBYTE posOffset;

    Read(fh, buf, 3);
    if (strcmp(buf, "NON")) goto fail;
    Read(fh, &compatibilityVersion, 1);

    switch (compatibilityVersion) {
      case 0:
        Read(fh, &fileVersion, 1);
        Read(fh, &fileRelease, 1);
        Read(fh, &numSizes, 1);
        if (!numSizes) goto fail;
        if ((sizes = AllocPooled(memoryPool, numSizes * (sizeof(UBYTE) + sizeof(UWORD))))) {
          ULONG i;
          numPuzzles = (UWORD*) (sizes + numSizes);

          for (i = 0; i < numSizes; i++) {
            Read(fh, &sizes[i], 1);
          }
          for (i = 0; i < numSizes; i++) {
            Read(fh, &numPuzzles[i], 2);
          }

          posOffset = Seek(fh, 0, OFFSET_BEGINNING);
        }
        else goto fail;

        pack->name = name;
        pack->fh = fh;
        pack->version = fileVersion;
        pack->release = fileRelease;
        pack->num_sizes = numSizes;
        pack->sizes = sizes;
        pack->num_Puzzles = numPuzzles;
        pack->pos_offset = posOffset;
      break;

      default:
      goto fail;
    }
  }

  return pack;
fail:
  FreePooled(memoryPool, pack, sizeof(Pack));
  return NULL;
}

VOID freePack(Pack* pack) {
  if (pack) {
    freeString(pack->name);
    Close(pack->fh);
    FreePooled(memoryPool, pack->sizes, pack->num_sizes * (sizeof(UBYTE) + sizeof(UWORD)));

    FreePooled(memoryPool, pack, sizeof(Pack));
  }
}

struct Packs* loadPacks(STRPTR dirPath) {
  struct Packs* packs = AllocPooled(memoryPool, sizeof(struct Packs));

  if (packs) {
    BPTR lock = Lock(dirPath, ACCESS_READ);
    packs->count = 0;
    NewList((struct List*) &packs->list);

    if (lock) {
      struct FileInfoBlock* fib = AllocDosObject(DOS_FIB, TAG_END);
      if (fib) {
        if (Examine(lock, fib)) {
          while (ExNext(lock, fib)) {
            if (fib->fib_DirEntryType < 0) {
              STRPTR name = stripExtension(fib->fib_FileName);
              if (name) {
                STRPTR filePath = makePath(dirPath, fib->fib_FileName, NULL);
                if (filePath) {
                  BPTR fh = Open(filePath, MODE_OLDFILE);
                  if (fh) {
                    Pack* pack = newPack(name, fh);
                    if (pack) {
                      AddTail((struct List*) &packs->list, (struct Node*) pack);
                      packs->count++;
                    }
                    else {
                      Close(fh);
                    }
                  }
                  freeString(filePath);
                }
              }
            }
          }
        }
        FreeDosObject(DOS_FIB, fib);
      }
      UnLock(lock);
    }
  }

  return packs;
}

VOID freePacks(struct Packs* packs) {
  Pack *p, *next;

  for (p = (Pack*) packs->list.mlh_Head; (next = (Pack*) p->node.mln_Succ); p = next) {
    freePack(p);
  }

  FreePooled(memoryPool, packs, sizeof(struct Packs));
}
