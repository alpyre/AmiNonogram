/******************************************************************************
 This file is part of AmiNonogram.
 Copyright (C) 2022 Ibrahim Alper Sönmez
******************************************************************************/

#ifndef PROFILE_H
#define PROFILE_H

#include "bitfield.h"
#include "pack.h"

struct Profiles {
  ULONG count;
  struct MinList list;
};

struct ProfileFile {
  struct MinNode node;
  STRPTR name;
  STRPTR fileName;
  LONG score;
  UBYTE offset;
};

typedef struct {
  STRPTR name;
  STRPTR fileName;
  LONG score;
  UWORD num_packStates;
  Bitfield **packStates;
} Profile;

struct ProfileFile* newProfileFile(STRPTR name, STRPTR dirPath);
struct ProfileFile* addProfileFile(struct Profiles* profiles, STRPTR name, STRPTR dirPath);
VOID removeProfileFile(struct Profiles* profiles, struct ProfileFile* pf, STRPTR dirPath);
struct Profiles* readProfileFiles(STRPTR dirPath);
VOID freeProfileFiles(struct Profiles* profiles);
Profile* newProfile(struct ProfileFile *pf, struct Packs *packs, STRPTR dirPath);
BOOL saveProfile(Profile* profile, STRPTR dirPath, struct Packs *packs);
VOID freeProfile(Profile* profile);

#endif //PROFILE_H

/******************************************************************************
 * FILE FORMAT                                                                *
 ******************************************************************************
 * HEADER             *
 **********************
 UBYTE header[3] = "PRO"
 UBYTE compatibilityVersion
 UBYTE[] name //NULL-TERMINATED ARRAY
 LONG score
 UWORD num_packStates
 **********************
 * PackStateData[]    *
 **********************
   UBYTE[] packName //NULL-TERMINATED ARRAY
   UBYTE version
   UBYTE release
   ULONG bitfieldSize //inBits (stands for numPuzzles)
   UBYTE bitfield[{BF_BYTESIZE(bitfieldSize)}]
*/
