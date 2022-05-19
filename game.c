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


#if defined(__amigaos4__)
#include <exec/exec.h>
#include <dos/dos.h>
//#include <dos/obsolete.h>
#include <intuition/intuition.h>
#include <intuition/iobsolete.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <time.h>
#else
#include <exec/exec.h>
#include <dos/dos.h>
#include <dos/dostags.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <clib/alib_protos.h>
#endif

#include "utility.h"
#include "game.h"
///
///definitions
#define UNDEFINED 0xFFFFFFFF
#define decrement(a) if (a) --a
///
///globals
extern APTR memoryPool;
///

/******************************************************************************
 * Takes the board size (as number of squares in a row/column) returns 0 if   *
 * it is an invalid value.                                                    *
 ******************************************************************************/
BOOL sizeCheck(ULONG size) {
  if (size % 5) {
    return FALSE;
  }
  else
    return TRUE;
}

GameBoardData *newGameBoardData(STRPTR name, ULONG size, ULONG pack_index, ULONG puzzle_index) {
  GameBoardData *gbd = NULL;

  if (sizeCheck(size)) {
    ULONG allocSize = sizeof(GameBoardData) + strlen(name) + 1 + (size * size);
    gbd = AllocPooled(memoryPool, allocSize);
    if (gbd) {
      gbd->size = size;
      gbd->pack_index = pack_index;
      gbd->puzzle_index = puzzle_index;
      gbd->name = (STRPTR)((UBYTE*)gbd + sizeof(GameBoardData));
      gbd->data = (UBYTE*)gbd + sizeof(GameBoardData) + strlen(name) + 1;
      memset(gbd->data, 0, size * size);

      strcpy(gbd->name, name);
    }
  }

  return gbd;
}

VOID freeGameBoardData(GameBoardData *gbd) {
  if (gbd) {
    ULONG allocSize = sizeof(GameBoardData) + (gbd->size * gbd->size) + strlen(gbd->name) + 1;
    FreePooled(memoryPool, (APTR)gbd, allocSize);
  }
}

#if defined(__amigaos4__)
ULONG rng(ULONG maxValue) {
  struct RandomState state;
  state.rs_High = time(NULL);
  state.rs_Low  = time(NULL);

  return Random(&state) % maxValue;
}
#else
ULONG rng(ULONG maxValue) {
  ULONG secs;
  ULONG mics;

  CurrentTime(&secs, &mics);
  return FastRand(secs ^ mics) % maxValue;
}
#endif

/******************************************************************************
 * Converts the planar data in bitfields and writes them into the given       *
 * GameBoardData.                                                             *
 ******************************************************************************/
VOID bf2gbd(GameBoardData* gbd, UBYTE plane, Bitfield* bf) {
  ULONG i;
  UBYTE mask = 0x01 << plane;

  for (i = 0; i < bf->size; i++) {
    if (checkBit(bf, i)) {
      gbd->data[i] |= mask;
    }
  }
}

ULONG bringPuzzle(struct Packs* packs, Pack* pack, Profile* profile, UBYTE size, GameBoardData** gbd) {
  ULONG i;
  ULONG pack_index = UNDEFINED;
  ULONG puzzle_index = UNDEFINED;
  ULONG puzzle_offset;
  ULONG start = 0;
  ULONG end = 0;
  ULONG rnd;
  Bitfield* bf;
  STRPTR puzzle_name;
  UBYTE puzzle_planes;

  *gbd = NULL;

  if (!packs->count) {
    return BP_NO_PACKS;
  }

  if (!pack) {
    rnd = rng(profile->num_packStates);

    if (size) {
      BOOL size_found = FALSE;

      for (pack = (Pack*) packs->list.mlh_Head, i = 0; i < profile->num_packStates; pack = (Pack*) pack->node.mln_Succ, i++) {
        ULONG size_index = UNDEFINED;
        ULONG start = 0;
        ULONG end;
        ULONG j;

        //Is this size available in this pack?
        for (j = 0; j < pack->num_sizes; j++) {
          if (size == pack->sizes[j]) {
            size_index = j;
            break;
          }
          start += pack->num_Puzzles[j];
        }

        if (size_index != UNDEFINED) {
          size_found = TRUE;
          end = start + pack->num_Puzzles[size_index];
          if (!isRangeAllSet(profile->packStates[i], start, end)) {
            pack_index = i;
            if (!rnd) break;
          }
        }
        decrement(rnd);
      }

      if (pack_index == UNDEFINED) {
        if (size_found) {
          //This size is solved in all packs
          return BP_SIZE_SOLVED_IN_ALL;
        }
        else {
          //No such size in any of the packs
          return BP_NO_SUCH_SIZE_IN_PACKS;
        }
      }
    }
    else {
      for (pack = (Pack*) packs->list.mlh_Head, i = 0; i < profile->num_packStates; pack = (Pack*) pack->node.mln_Succ, i++) {
        if (!isAllSet(profile->packStates[i])) {
          pack_index = i;
          if (!rnd) break;
        }
        decrement(rnd);
      }

      if (pack_index == UNDEFINED) {
        //All packs have been solved!
        return BP_ALL_PACKS_SOLVED;
      }
    }
  }
  else {
    Pack* p;
    for (p = (Pack*) packs->list.mlh_Head, pack_index = 0; p != pack; p = (Pack*) pack->node.mln_Succ, pack_index++);
  }

  if (!size) {
    for (i = 0; i < pack->num_sizes; i++) {
      end += pack->num_Puzzles[i];
    }
  }
  else {
    ULONG size_index = UNDEFINED;
    //find the requested size
    for (i = 0; i < pack->num_sizes; i++) {
      if (pack->sizes[i] == size) {
        size_index = i;
        break;
      }
    }

    if (size_index == UNDEFINED) {
      //There is no such size in this pack!
      return BP_NO_SUCH_SIZE_IN_PACK;
    }

    for (i = 0; i < size_index; i++) {
      start += pack->num_Puzzles[i];
    }

    end = start + pack->num_Puzzles[size_index];
  }

  rnd = rng(end - start);

  for (i = start; i < end; i++) {
    if (!checkBit(profile->packStates[pack_index], i)) {
      puzzle_index = i;
      if (!rnd) break;
    }
    decrement(rnd);
  }

  if (puzzle_index == UNDEFINED) {
    //All the puzzles of this size in this pack are solved!
    return BP_SIZE_SOLVED_IN_THIS;
  }

  if (!size) {
    //find the size of this puzzle
    ULONG size_index = 0;
    ULONG total = 0;

    for (i = 0; i < pack->num_sizes; i++) {
      total += pack->num_Puzzles[i];
      if (puzzle_index < total) {
        size_index = i;
        break;
      }
    }

    size = pack->sizes[size_index];
  }

  //read the planar data from the file into gbd
  Seek(pack->fh, pack->pos_offset + 4 * puzzle_index, OFFSET_BEGINNING);
  Read(pack->fh, &puzzle_offset, 4);
  Seek(pack->fh, puzzle_offset, OFFSET_BEGINNING);

  puzzle_name = readString(pack->fh);
  if (puzzle_name) {
    *gbd = newGameBoardData(puzzle_name, size, pack_index, puzzle_index);
    if (*gbd) {
      Read(pack->fh, &puzzle_planes, 1);
      for (i = 0; i < puzzle_planes; i++) {
        bf = newBitfield(size * size);
        if (bf) {
          Read(pack->fh, bf->field, BF_BYTESIZE(bf->size));
          bf2gbd(*gbd, i, bf);
          freeBitfield(bf);
        }
        else {
          freeGameBoardData(*gbd);
          *gbd = NULL;
          return BP_MEMORY_ERROR;
        }
      }
    }
    else return BP_MEMORY_ERROR;
  }
  else return BP_MEMORY_ERROR;

  return 0;
}

/* DEBUG
VOID printGBD(GameBoardData *gbd) {
  ULONG r, c;

  printf("%s\n", gbd->name);
  printf("%lu\n", gbd->size);
  for (r = 0; r < gbd->size; r++) {
    for (c = 0; c < gbd->size; c++) {
      printf("%i, ", gbd->data[r * gbd->size + c]);
    }
    printf("\n");
  }
}
*/
