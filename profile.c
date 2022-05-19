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

/******************************************************************************
 * TODO: A lot of effort has been made to guarantee failsafe memory           *
 * allocations, though there are very few checks implemented for filesystem   *
 * read/write failures.                                                       *
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

#include "profile.h"
#include "utility.h"
///
///globals
extern APTR memoryPool;
///

BOOL isValidProfileFile(BPTR fh) {
  UBYTE buf[4] = {0};
  UBYTE compatibilityVersion;

  Read(fh, buf, 3);
  if (strcmp(buf, "PRO")) return FALSE;
  Read(fh, &compatibilityVersion, 1);
  if (compatibilityVersion != 0) return FALSE;

  return TRUE;
}

VOID stripRestricted(STRPTR name) {
  STRPTR restricted = "#?~'\"<>|[]";
  UBYTE* r;
  UBYTE* c = name;

  while (*c) {
    r = restricted;
    while (*r) {
      if (*c == *r) {
        *c = '_';
        break;
      }
      r++;
    }
    c++;
  }
}

STRPTR generateFileName(STRPTR name, STRPTR dirPath) {
  STRPTR path = NULL;
  STRPTR fName = makeString(name);

  if (fName) {
    stripRestricted(fName);
    path = makePath(dirPath, fName, NULL);
    if (path) {
      ULONG i = 1;
      while (Exists(path)) {
        UBYTE buffer[16];
        STRPTR oldFName = fName;

        sprintf(buffer, "_%lu", i);
        fName = makeString2(fName, buffer);
        freeString(oldFName);
        freeString(path);
        path = makePath(dirPath, fName, NULL);
        if (!path) goto fail_3;
        i++;
      }
      freeString(path);
    }
    else goto fail_3;
  }

  return fName;
fail_3:
  if (path) freeString(path);
  freeString(fName);
  return NULL;
}

struct ProfileFile* newProfileFile(STRPTR name, STRPTR dirPath) {
  struct ProfileFile* pf = AllocPooled(memoryPool, sizeof(struct ProfileFile));
  STRPTR fName = NULL;
  STRPTR path = NULL;

  if (pf) {
    fName = generateFileName(name, dirPath);
    if (fName) {
      path = makePath(dirPath, fName, NULL);
      if (path) {
        BPTR fh = Open(path, MODE_NEWFILE);
        if (fh) {
          UWORD num_packStates = 0;
          UBYTE compatibilityVersion = 0;
          LONG score = 0;

          Write(fh, "PRO", 3);
          Write(fh, &compatibilityVersion, 1);
          Write(fh, name, strlen(name) + 1);
          Write(fh, &score, 4);
          Write(fh, &num_packStates, 2);

          pf->name = makeString(name);
          pf->fileName = fName;
          pf->score = score;
          pf->offset = Seek(fh, 0, OFFSET_CURRENT) - 2;

          Close(fh);
        }
        else goto fail_4;

        freeString(path);
      }
      else goto fail_4;
    }
    else goto fail_4;
  }

  return pf;
fail_4:
  if (path) freeString(path);
  if (fName) freeString(fName);
  if (pf) FreePooled(memoryPool, pf, sizeof(struct ProfileFile));

  return NULL;
}

struct ProfileFile* readProfileFile(BPTR fh, STRPTR filename) {
  struct ProfileFile* p = AllocPooled(memoryPool, sizeof(struct ProfileFile));
  STRPTR name = NULL;
  STRPTR fileName = NULL;
  LONG score = 0;

  Seek(fh, 4, OFFSET_BEGINNING);

  if (p) {
    if ((name = readString(fh))) {
      if ((fileName = makeString(filename))) {
        Read(fh, &score, 4);
        p->name = name;
        p->fileName = fileName;
        p->score = score;
        p->offset = Seek(fh, 0, OFFSET_CURRENT);
      }
      else goto fail_1;
    }
    else goto fail_1;
  }

  return p;
fail_1:
  if (name) freeString(name);
  FreePooled(memoryPool, p, sizeof(struct ProfileFile));
  return NULL;
}

VOID freeProfileFile(struct ProfileFile* p) {
  if (p) {
    freeString(p->name);
    freeString(p->fileName);
    FreePooled(memoryPool, p, sizeof(struct ProfileFile));
  }
}

struct Profiles* readProfileFiles(STRPTR dirPath) {
  struct Profiles* profiles = AllocPooled(memoryPool, sizeof(struct Profiles));

  if (profiles) {
    BPTR lock = Lock(dirPath, ACCESS_READ);
    profiles->count = 0;
    NewList((struct List*) &profiles->list);

    if (lock) {
      struct FileInfoBlock* fib = AllocDosObject(DOS_FIB, TAG_END);
      if (fib) {
        if (Examine(lock, fib)) {
          while (ExNext(lock, fib)) {
            if (fib->fib_DirEntryType < 0) {
              STRPTR filePath = makePath(dirPath, fib->fib_FileName, NULL);
              if (filePath) {
                BPTR fh = Open(filePath, MODE_OLDFILE);
                if (fh) {
                  if (isValidProfileFile(fh)) {
                    struct ProfileFile* pf = readProfileFile(fh, fib->fib_FileName);
                    if (pf) {
                      AddHead((struct List*) &profiles->list, (struct Node *) pf);
                      profiles->count++;
                    }
                  }
                  Close(fh);
                }
                freeString(filePath);
              }
            }
          }
        }
        FreeDosObject(DOS_FIB, fib);
      }
      UnLock(lock);
    }
  }

  return profiles;
}

VOID freeProfileFiles(struct Profiles* profiles) {
  if (profiles) {
    struct ProfileFile *pf, *next;
    for (pf = (struct ProfileFile*) profiles->list.mlh_Head; (next = (struct ProfileFile*) pf->node.mln_Succ); pf = next) {
      freeProfileFile(pf);
    }

    FreePooled(memoryPool, profiles, sizeof(struct Profiles));
  }
}

VOID getPackStateData(BPTR fh, Pack* pack, ULONG totalPuzzles, Bitfield* bf, UBYTE offset) {
  ULONG i;
  UWORD num_packStates;

  Seek(fh, offset, OFFSET_BEGINNING);
  Read(fh, &num_packStates, 2);

  for (i = 0; i < num_packStates; i++) {
    STRPTR name = readString(fh);
    UBYTE version;
    UBYTE release;
    ULONG bfSize;

    Read(fh, &version, 1);
    Read(fh, &release, 1);
    Read(fh, &bfSize, 4);

    if (name) {
      if (!strcmp(name, pack->name)) {
        if (version == pack->version && bfSize <= totalPuzzles) {
          Read(fh, bf->field, BF_BYTESIZE(bfSize));
        }
        freeString(name);
        break;
      }

      Seek(fh, bfSize, OFFSET_CURRENT);
      freeString(name);
    }
    else break;
  }
}

LONG getScore(BPTR fh, UBYTE offset) {
  LONG score;

  Seek(fh, offset, OFFSET_BEGINNING);
  Read(fh, &score, 4);

  return score;
}

Profile* newProfile(struct ProfileFile *pf, struct Packs *packs, STRPTR dirPath) {
  Profile *profile = AllocPooled(memoryPool, sizeof(Profile));
  Bitfield **packStates = NULL;
  Pack *pack;
  ULONG i, j;
  ULONG totalPuzzles;
  LONG score = 0;
  STRPTR path = NULL;
  BPTR fh = 0;

  if (profile) {

    if (packs->count) {
      if ((packStates = AllocPooled(memoryPool, packs->count * sizeof(Bitfield *)))) {
        path = makePath(dirPath, pf->fileName, NULL);
        if (path) {
          fh = Open(path, MODE_OLDFILE);

          for (pack = (Pack*) packs->list.mlh_Head, i = 0; pack->node.mln_Succ; pack = (Pack*) pack->node.mln_Succ, i++) {
            totalPuzzles = 0;
            for (j = 0; j < pack->num_sizes; j++) totalPuzzles += pack->num_Puzzles[j];

            packStates[i] = newBitfield(totalPuzzles);
            if (!packStates[i]) goto fail_2;

            if (fh) {
              getPackStateData(fh, pack, totalPuzzles, packStates[i], pf->offset);
            }
          }

          if (fh) {
            score = getScore(fh, pf->offset - 4);
            Close(fh);
          }
          freeString(path);
        }
        else goto fail_2;

      }
      else goto fail_2;
    }
    profile->name = pf->name;
    profile->fileName = pf->fileName;
    profile->score = score;
    profile->num_packStates = packs->count;
    profile->packStates = packStates;
  }

  return profile;

fail_2:
  printf("Memory Error\n");
  if (fh) Close(fh);
  if (path) freeString(path);
  if (packStates) {
    for (i = 0; i < packs->count; i++) {
      if (packStates[i]) freeBitfield(packStates[i]);
      else break;
    }
    FreePooled(memoryPool, packStates, packs->count * sizeof(Bitfield*));
  }
  FreePooled(memoryPool, profile, sizeof(Profile));
  return NULL;
}

VOID freeProfile(Profile* profile) {
  if (profile) {
    ULONG i;

    for (i = 0; i < profile->num_packStates; i++) {
      freeBitfield(profile->packStates[i]);
    }
    FreePooled(memoryPool, profile->packStates, profile->num_packStates * sizeof(Bitfield*));
    FreePooled(memoryPool, profile, sizeof(Profile));
  }
}

BOOL saveProfile(Profile* profile, STRPTR dirPath, struct Packs *packs) {
  BOOL _success = FALSE;
  STRPTR path = makePath(dirPath, profile->fileName, NULL);

  if (path) {
    BPTR fh = Open(path, MODE_READWRITE);
    if (fh) {
      Pack* pack;
      ULONG i;
      UBYTE compatibilityVersion = 0;

      Write(fh, "PRO", 3);
      Write(fh, &compatibilityVersion, 1);
      Write(fh, profile->name, strlen(profile->name) + 1);
      Write(fh, &profile->score, 4);
      Write(fh, &profile->num_packStates, 2);

      for (pack = (Pack*) packs->list.mlh_Head, i = 0; pack->node.mln_Succ; pack = (Pack*) pack->node.mln_Succ, i++) {
        Write(fh, pack->name, strlen(pack->name) + 1);
        Write(fh, &pack->version, 1);
        Write(fh, &pack->release, 1);
        Write(fh, &profile->packStates[i]->size, 4);
        Write(fh, profile->packStates[i]->field, BF_BYTESIZE(profile->packStates[i]->size));
      }

      SetFileSize(fh, 0, OFFSET_CURRENT);
      Close(fh);
      _success = TRUE;
    }

    freeString(path);
  }

  return _success;
}

struct ProfileFile* addProfileFile(struct Profiles* profiles, STRPTR name, STRPTR dirPath) {
  struct ProfileFile* pf = newProfileFile(name, dirPath);
  if (pf) {
    AddTail((struct List*) &profiles->list, (struct Node*) pf);
    profiles->count++;
  }
  return pf;
}

VOID removeProfileFile(struct Profiles* profiles, struct ProfileFile* pf, STRPTR dirPath) {
  STRPTR path = makePath(dirPath, pf->fileName, NULL);

  if (path) {
    if (Exists(path)) DeleteFile(path);
    Remove((struct Node*) pf);
    profiles->count--;
    freeProfileFile(pf);
    freeString(path);
  }
}
