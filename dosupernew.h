/******************************************************************************
 This file is part of AmiNonogram.
 Copyright (C) 2022 Ibrahim Alper Sönmez
******************************************************************************/

#ifndef DO_SUPER_NEW_H
#define DO_SUPER_NEW_H

#if !defined(__MORPHOS__)
Object* VARARGS68K DoSuperNew(struct IClass *cl, Object *obj, ...);
#endif

#endif //DO_SUPER_NEW_H
