/******************************************************************************
 This file is part of AmiNonogram.
 Copyright (C) 2022 Ibrahim Alper Sönmez
******************************************************************************/

#ifndef UTILITY_H
#define UTILITY_H

BOOL Exists(STRPTR filename);
STRPTR readString(BPTR fh);
STRPTR makePath(STRPTR dir, STRPTR file, STRPTR extension);
STRPTR stripExtension(STRPTR filename);
STRPTR makeString(STRPTR str);
STRPTR makeString2(STRPTR str1, STRPTR str2);
//STRPTR makeString3(STRPTR str1, STRPTR str2, STRPTR str3);
VOID freeString(STRPTR str);

#endif //UTILITY_H
