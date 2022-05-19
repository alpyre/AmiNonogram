/******************************************************************************
 This file is part of AmiNonogram.
 Copyright (C) 2022 Ibrahim Alper Sönmez
******************************************************************************/

#ifndef GAME_H
#define GAME_H

#include "profile.h"
#include "pack.h"

#define BP_MEMORY_ERROR           1
#define BP_NO_PACKS              10
#define BP_ALL_PACKS_SOLVED      11
#define BP_NO_SUCH_SIZE_IN_PACKS 12
#define BP_NO_SUCH_SIZE_IN_PACK  13
#define BP_SIZE_SOLVED_IN_ALL    14
#define BP_SIZE_SOLVED_IN_THIS   15

typedef struct {
  STRPTR name; //name of the puzzle
  ULONG size;  //number of columns
  ULONG pack_index;
  ULONG puzzle_index;
  UBYTE *data; //every pixel on the board is a byte
} GameBoardData;

GameBoardData *newGameBoardData(STRPTR name, ULONG size, ULONG pack_index, ULONG puzzle_index);
VOID freeGameBoardData(GameBoardData *gbd);
ULONG bringPuzzle(struct Packs* packs, Pack* pack, Profile* profile, UBYTE size, GameBoardData** gbd);
//DEBUG: VOID printGBD(GameBoardData *gbd);

#endif //GAME_H
