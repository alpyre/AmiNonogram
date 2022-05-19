/******************************************************************************
 This file is part of AmiNonogram.
 Copyright (C) 2022 Ibrahim Alper Sönmez
******************************************************************************/

#ifndef BITFIELD_H
#define BITFIELD_H

#define BF_BYTESIZE(s) ((s)/8+(((s)%8)?1:0))

typedef struct {
  ULONG size; //in bits
  UBYTE *field;
} Bitfield;

Bitfield* newBitfield(ULONG size);
VOID freeBitfield(Bitfield* bf);
VOID setBit(Bitfield* bf, ULONG index);
VOID clearBit(Bitfield* bf, ULONG index);
BOOL checkBit(Bitfield* bf, ULONG index);
BOOL isAllSet(Bitfield* bf);
BOOL isRangeAllSet(Bitfield* bf, ULONG start, ULONG end);

#endif //BITFIELD_H
