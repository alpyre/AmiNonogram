/******************************************************************************
 This file is part of AmiNonogram.
 Copyright (C) 2022 Ibrahim Alper Sönmez
******************************************************************************/

#include <exec/exec.h>
#include <proto/exec.h>
#include "bitfield.h"

extern APTR memoryPool;

Bitfield* newBitfield(ULONG size) {
  Bitfield* bf = NULL;
  ULONG byteSize = BF_BYTESIZE(size);
  UBYTE *field = AllocPooled(memoryPool, byteSize);
  memset(field, 0, byteSize); // Clear if not cleared in allocation

  if (field) {
    bf = AllocPooled(memoryPool, sizeof(Bitfield));
    if (bf) {
      bf->size = size;
      bf->field = field;
    }
    else FreePooled(memoryPool, field, byteSize);
  }

  return bf;
}

VOID freeBitfield(Bitfield* bf) {
  if (bf) {
    FreePooled(memoryPool, bf->field, BF_BYTESIZE(bf->size));
    FreePooled(memoryPool, bf, sizeof(Bitfield));
  }
}

VOID setBit(Bitfield* bf, ULONG index) {
  ULONG byteIndex = index / 8;
  ULONG bitIndex  = index % 8;
  UBYTE mask = 0x80 >> bitIndex;

  /*
  if (index > bf->size) {
    printf("Invalid index!");
    return;
  }
  */

  bf->field[byteIndex] |= mask;
}

VOID clearBit(Bitfield* bf, ULONG index) {
  ULONG byteIndex = index / 8;
  ULONG bitIndex  = index % 8;
  UBYTE mask = 0x80 >> bitIndex;

  /*
  if (index > bf->size) {
    printf("Invalid index!");
    return;
  }
  */

  bf->field[byteIndex] &= ~mask;
}

BOOL checkBit(Bitfield* bf, ULONG index) {
  ULONG byteIndex = index / 8;
  ULONG bitIndex  = index % 8;
  UBYTE mask = 0x80 >> bitIndex;

  /*
  if (index > bf->size) {
    printf("Invalid index!");
    return;
  }
  */

  return (bf->field[byteIndex] & mask);
}

BOOL isAllSet(Bitfield* bf) {
  ULONG i;
  ULONG byteCount = bf->size / 8;
  ULONG bitCount = bf->size % 8;
  UBYTE mask = 0xFF << (8 - bitCount);

  for (i = 0; i < byteCount; i++) {
    if (bf->field[i] != 0xFF) {
      return FALSE;
    }
  }
  if (bitCount && bf->field[byteCount] != mask) {
    return FALSE;
  }

  return TRUE;
}

BOOL isRangeAllSet(Bitfield* bf, ULONG start, ULONG end) {
  ULONG i;
  ULONG startByte = start / 8;
  ULONG endByte = end / 8;
  ULONG byteCount = endByte - startByte;
  UBYTE startMask = 0xFF >> (start % 8);
  UBYTE endMask = 0xFF << (8 - ((end + 1) % 8));

  if (!byteCount) {
    UBYTE mask = (startMask & endMask);
    if ((bf->field[startByte] & mask) != mask) {
      return FALSE;
    }
  }
  else {
    if ((bf->field[startByte] & startMask) != startMask) {
      return FALSE;
    }
    for (i = startByte + 1; i < endByte; i++) {
      if (bf->field[i] != 0xFF) {
        return FALSE;
      }
    }
    if ((bf->field[endByte] & endMask) != endMask) {
      return FALSE;
    }
  }

  return TRUE;
}
