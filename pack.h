/******************************************************************************
 This file is part of AmiNonogram.
 Copyright (C) 2022 Ibrahim Alper Sönmez
******************************************************************************/

#ifndef PACK_H
#define PACK_H

struct Packs {
  ULONG count;
  struct MinList list;
};

typedef struct {
  struct MinNode node;
  STRPTR name;
  BPTR fh;
  UBYTE version;
  UBYTE release;
  UBYTE *sizes;
  UWORD *num_Puzzles;
  UBYTE num_sizes;
  UBYTE pos_offset;
} Pack;

struct Packs* loadPacks(STRPTR dir);
VOID freePacks(struct Packs* packs);

#endif //PACK_H

/******************************************************************************
 * FILE FORMAT                                                                *
 ******************************************************************************
 * HEADER             *
 **********************
 UBYTE header[3] = "NON"
 UBYTE compatibilityVersion
 UBYTE fileVersion
 UBYTE fileRelease
 UBYTE numberOfSizes
 **********************
 * PUZZLE INFORMATION *
 **********************
 UBYTE sizes[numberOfSizes]
 UWORD numPuzzles[numberOfSizes]
 ULONG puzzleOffsets[totalNumberOfPuzzles]
 **********************
 * PUZZLES[]          *
 **********************
   UBYTE[] name //NULL-TERMINATED ARRAY
   UBYTE planes
   UBYTE[{BF_BYTESIZE(puzzleSize)} * planes] data
*/
