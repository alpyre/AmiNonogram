/******************************************************************************
 * LivesDisplay                                                               *
 ******************************************************************************/
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

///Includes
//standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include <proto/exec.h>
#include <proto/utility.h>    // <-- Required for tag redirection
#include <graphics/gfx.h>

#include <libraries/mui.h>
#include <proto/muimaster.h>
#include <clib/alib_protos.h> // <-- Required for DoSuperMethod()

#include <SDI_compiler.h>     //     Required for
#include <SDI_hook.h>         // <-- multi platform
#include <SDI_stdarg.h>       //     compatibility

#include "dosupernew.h"
#include "livesdisplay.h"
///
///Structs
struct cl_Data
{
  Object **hearts; //An array of the bitmap objects which display each life
  ULONG max_lives; //The number of the above bitmap objects
  ULONG lives;     //The number of lives the player has currently
};

struct cl_Msg
{
  ULONG MethodID;
  //<SUBCLASS METHOD MESSAGE PAYLOAD HERE>
};
///
///Globals
extern APTR memoryPool;
#define HEART_IMAGE_SIZE 13 // Heart image is a 13x13 square image
static UWORD bm_heartEmpty[HEART_IMAGE_SIZE] = {0x38E0, 0x4510, 0x8208, 0x8008, 0x8008, 0x8008,
                      0x4010, 0x4010, 0x2020, 0x1040, 0x880, 0x500, 0x200};

static UWORD bm_heartFilled[HEART_IMAGE_SIZE] = {0x38E0, 0x7DF0, 0xFFF8, 0xFFF8, 0xFFF8, 0xFFF8,
                      0x7FF0, 0x7FF0, 0x3FE0, 0x1FC0, 0xF80, 0x700, 0x200};

static struct BitMap heartEmpty = {2, HEART_IMAGE_SIZE, BMF_STANDARD, 1, 0, {(PLANEPTR)bm_heartEmpty}};
static struct BitMap heartFilled = {2, HEART_IMAGE_SIZE, BMF_STANDARD, 1, 0, {(PLANEPTR)bm_heartFilled}};
///

//<SUBCLASS METHODS>
///m_LoseLife(cl, obj)
static ULONG m_LoseLife(struct IClass* cl, Object* obj)
{
  struct cl_Data *data = INST_DATA(cl, obj);

  if (data->lives > 0) {
    DoMethod(obj, MUIM_Set, MUIA_LivesDisplay_Lives, data->lives - 1);
    return TRUE;
  }

  return FALSE;
}
///

///Overridden OM_NEW
static ULONG m_New(struct IClass* cl, Object* obj, struct opSet* msg)
{
  struct cl_Data *data;
  ULONG max_lives;
  ULONG lives;
  Object **hearts;
  struct TagItem *tagList;
  struct TagItem *tag;

  max_lives = GetTagData(MUIA_LivesDisplay_MaxLives, 3, msg->ops_AttrList);
  lives = GetTagData(MUIA_LivesDisplay_Lives, max_lives, msg->ops_AttrList);
  if (lives > max_lives) lives = max_lives;

  hearts = AllocPooled(memoryPool, sizeof(Object*) * max_lives);
  if (hearts) {
    tagList = AllocPooled(memoryPool, sizeof(struct TagItem) * (max_lives + 1));
    if (tagList) {
      ULONG i;
      tag = tagList;
      for (i = 0; i < max_lives; i++) {
        tag->ti_Tag = MUIA_Group_Child;
        hearts[i] = MUI_NewObject(MUIC_Bitmap,
          MUIA_Bitmap_Bitmap, i >= lives ? (ULONG)&heartEmpty : (ULONG)&heartFilled,
          MUIA_Bitmap_Width, HEART_IMAGE_SIZE,
          MUIA_FixWidth, HEART_IMAGE_SIZE,
          MUIA_Bitmap_Height, HEART_IMAGE_SIZE,
          MUIA_FixHeight, HEART_IMAGE_SIZE,
          MUIA_Bitmap_Transparent, 0,
          MUIA_Bitmap_UseFriend, TRUE,
          MUIA_UserData, i,
          TAG_END);
        tag->ti_Data = (ULONG) hearts[i];
        if (!tag->ti_Data) {
          tag = tagList;
          while (tag->ti_Data) {
            MUI_DisposeObject((Object*)tag->ti_Data);
            tag++;
          }
          FreePooled(memoryPool, tagList, sizeof(struct TagItem) * (max_lives + 1));
          FreePooled(memoryPool, hearts, sizeof(Object*) * max_lives);
          return NULL;
        }
        tag++;
      }
      tag->ti_Tag = TAG_MORE;
      tag->ti_Data = (ULONG)msg->ops_AttrList;
    }
    else {
      FreePooled(memoryPool, hearts, sizeof(Object*) * max_lives);
      return NULL;
    }
  }
  else return NULL;

  if ((obj = (Object *) DoSuperNew(cl, obj,
    MUIA_Group_Horiz, TRUE,
    TAG_MORE, tagList)))
  {
    data = (struct cl_Data *) INST_DATA(cl, obj);

    data->hearts = hearts;
    data->max_lives = max_lives;
    data->lives = lives;

    FreePooled(memoryPool, tagList, sizeof(struct TagItem) * (max_lives + 1));

    //<SUBCLASS INITIALIZATION HERE>
    if (/*<Success of your initializations>*/ TRUE) {

      return((ULONG) obj);
    }
    else CoerceMethod(cl, obj, OM_DISPOSE);
  }

return NULL;
}
///
///Overridden OM_DISPOSE
static ULONG m_Dispose(struct IClass* cl, Object* obj, Msg msg)
{
  struct cl_Data *data = INST_DATA(cl, obj);

  FreePooled(memoryPool, data->hearts, sizeof(Object*) * data->max_lives);

  return DoSuperMethodA(cl, obj, msg);
}
///
///Overridden OM_SET
//*****************
static ULONG m_Set(struct IClass* cl, Object* obj, struct opSet* msg)
{
  struct cl_Data *data = INST_DATA(cl, obj);
  struct TagItem *tags, *tag;

  for (tags = msg->ops_AttrList; (tag = NextTagItem(&tags));)
  {
    switch (tag->ti_Tag)
    {
      case MUIA_LivesDisplay_Lives:
      {
        ULONG i;

        if (tag->ti_Data > data->max_lives) tag->ti_Data = data->max_lives;

        if (data->lives > tag->ti_Data ) {
          for (i = tag->ti_Data; i < data->lives; i++)
            DoMethod(data->hearts[i], MUIM_Set, MUIA_Bitmap_Bitmap, &heartEmpty);
        }
        else {
          for (i = data->lives; i < tag->ti_Data; i++)
            DoMethod(data->hearts[i], MUIM_Set, MUIA_Bitmap_Bitmap, &heartFilled);
        }

        data->lives = tag->ti_Data;
      }
      break;
    }
  }

  return (DoSuperMethodA(cl, obj, (Msg) msg));
}
///
///Overridden OM_GET
//*****************
static ULONG m_Get(struct IClass* cl, Object* obj, struct opGet* msg)
{
  struct cl_Data *data = INST_DATA(cl, obj);

  switch (msg->opg_AttrID)
  {
    case MUIA_LivesDisplay_MaxLives:
      *msg->opg_Storage = data->max_lives;
    return TRUE;
    case MUIA_LivesDisplay_Lives:
      *msg->opg_Storage = data->lives;
    return TRUE;
  }

  return (DoSuperMethodA(cl, obj, (Msg) msg));
}
///
///Dispatcher
SDISPATCHER(cl_Dispatcher)
{
  struct cl_Data *data;
  if (! (msg->MethodID == OM_NEW)) data = INST_DATA(cl, obj);

  switch(msg->MethodID)
  {
    case OM_NEW:
      return m_New(cl, obj, (struct opSet*) msg);
    case OM_DISPOSE:
      return m_Dispose(cl, obj, msg);
    case OM_SET:
      return m_Set(cl, obj, (struct opSet*) msg);
    case OM_GET:
      return m_Get(cl, obj, (struct opGet*) msg);
    case MUIM_LivesDisplay_LoseLife:
      return m_LoseLife(cl, obj);

    //<DISPATCH SUBCLASS METHODS HERE>

    default:
      return DoSuperMethodA(cl, obj, msg);
  }
}
///
///Class Creator
struct MUI_CustomClass* MUI_Create_LivesDisplay(void)
{
    return (MUI_CreateCustomClass(NULL, MUIC_Group, NULL, sizeof(struct cl_Data), ENTRY(cl_Dispatcher)));
}
///
