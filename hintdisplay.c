/******************************************************************************
 * HintDisplay                                                                *
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
#include <clib/alib_protos.h> // <-- Required for DoSuperMethod()

#include <libraries/mui.h>
#include <proto/muimaster.h>

#include <SDI_compiler.h>     //     Required for
#include <SDI_hook.h>         // <-- multi platform
#include <SDI_stdarg.h>       //     compatibility

#include "dosupernew.h"
#include "hintdisplay.h"
///
///Globals
extern APTR memoryPool;
extern struct Library *MUIMasterBase;
///
///Structs
struct cl_Data
{
  Object **hints;  //An array of the text objects which display each hint
  ULONG num_hints; //Number of the above text objects
};

struct cl_Msg
{
  ULONG MethodID;
  ULONG hint;    // Index of the hint to mark as revealed
  ULONG reverse; // BOOL: above index will be counted from the end of the array
};
///

//<SUBCLASS METHODS>
///m_Reveal(cl, obj, msg)
/*****************************************************************************
 * Marks a hint as revealed (makes it bold) in the hintDisplay with the      *
 * given index in msg->hint.                                                 *
 * If msg->reverse is TRUE the index will be counted from the end.           *
 * Ex: hint display has: [2 4 5 1]                                           *
 *     msg->hint == 0; msg->reverse == FALSE;                                *
 *       Hint with the value 2 will be revealed.                             *
 *     msg->hint == 1; msg->reverse == TRUE;                                 *
 *       Hint with the value 5 will be revealed.                             *
 *****************************************************************************/
static ULONG m_Reveal(struct IClass* cl, Object* obj, struct cl_Msg* msg)
{
  struct cl_Data* data = INST_DATA(cl, obj);
  ULONG hint = msg->reverse ? data->num_hints - msg->hint - 1 : msg->hint;

  DoMethod(data->hints[hint], MUIM_Set, MUIA_Text_PreParse, "\33b\33c");

  return TRUE;
}
///
///m_RevealAll
/*****************************************************************************
 * Marks all hints as revealed (makes them bold) in the hintDisplay.         *
 *****************************************************************************/
static ULONG m_RevealAll(struct IClass* cl, Object* obj, Msg msg)
{
  struct cl_Data* data = INST_DATA(cl, obj);
  ULONG i;

  for (i = 0; i < data->num_hints; i++) {
    DoMethod(data->hints[i], MUIM_Set, MUIA_Text_PreParse, "\33b\33c");
  }

  return TRUE;
}
///

///Overridden OM_NEW
static ULONG m_New(struct IClass* cl, Object* obj, struct opSet* msg)
{
  struct cl_Data *data;
  struct TagItem *tagList;
  struct TagItem *tag;
  Object **hints;
  STRPTR *hintsArray;
  ULONG arrLen = 0;

  if((tag = FindTagItem(MUIA_HintDisplay_HintArray, msg->ops_AttrList)) && (hintsArray = (STRPTR*)tag->ti_Data)) {
    while (hintsArray[arrLen++]); arrLen--; // get size of hintsArray

    hints = (Object **) AllocPooled(memoryPool, sizeof(Object*) * arrLen);
    if (hints) {
      tagList = (struct TagItem*) AllocPooled(memoryPool, sizeof(struct TagItem) * (arrLen + 1));
      if (tagList) {
        ULONG i;
        tag = tagList;
        for (i = 0; i < arrLen; i++) {
          tag->ti_Tag = MUIA_Group_Child;
          hints[i] = MUI_NewObject(MUIC_Text,
                                   MUIA_Text_PreParse, "\33c",
                                   MUIA_Text_Contents, hintsArray[i],
                                   MUIA_HorizWeight, 0,
                                   TAG_END);
          tag->ti_Data = (ULONG) hints[i];
          if (!tag->ti_Data) goto error;

          tag++;
        }
        tag->ti_Tag = TAG_MORE;
        tag->ti_Data = (ULONG)msg->ops_AttrList;
      }
      else {
        FreePooled(memoryPool, hints, sizeof(Object*) * arrLen);
        return NULL;
      }
    }
    else return NULL;
  }
  else return NULL;

  if ((obj = (Object *) DoSuperNew(cl, obj,
    MUIA_Background, MUII_ButtonBack,
    MUIA_Group_Child, MUI_NewObject(MUIC_Rectangle, TAG_END),
    TAG_MORE, tagList)))
  {
    data = (struct cl_Data*) INST_DATA(cl, obj);

    data->hints = hints;
    data->num_hints = arrLen;

    FreePooled(memoryPool, tagList, sizeof(struct TagItem) * (arrLen + 1));

    //<SUBCLASS INITIALIZATION HERE>
    if (/*<Success of your initializations>*/ TRUE) {

      return((ULONG) obj);
    }
    else CoerceMethod(cl, obj, OM_DISPOSE);
  }

error:
  tag = tagList;
  while (tag->ti_Data && tag->ti_Data != (ULONG)msg->ops_AttrList) {
    MUI_DisposeObject((Object*)tag->ti_Data);
    tag++;
  }
  FreeMem(tagList, sizeof(struct TagItem) * (arrLen + 1));
  return NULL;
}
///
///Overridden OM_DISPOSE
static ULONG m_Dispose(struct IClass* cl, Object* obj, Msg msg)
{
  struct cl_Data *data = INST_DATA(cl, obj);

  FreePooled(memoryPool, data->hints, sizeof(Object*) * data->num_hints);

  return DoSuperMethodA(cl, obj, msg);
}
///
///Dispatcher
SDISPATCHER(cl_Dispatcher)
{
  struct cl_Data* data;
  if (! (msg->MethodID == OM_NEW)) data = INST_DATA(cl, obj);

  switch(msg->MethodID)
  {
    case OM_NEW:
      return m_New(cl, obj, (struct opSet*) msg);
    case OM_DISPOSE:
      return m_Dispose(cl, obj, (Msg) msg);
    case MUIM_HintDisplay_Reveal:
      return m_Reveal(cl, obj, (struct cl_Msg*) msg);
    case MUIM_HintDisplay_RevealAll:
      return m_RevealAll(cl, obj, (Msg) msg);

    default:
      return DoSuperMethodA(cl, obj, msg);
  }
}
///
///Class Creator
struct MUI_CustomClass* MUI_Create_HintDisplay(void)
{
    return (MUI_CreateCustomClass(NULL, MUIC_Group, NULL, sizeof(struct cl_Data), ENTRY(cl_Dispatcher)));
}
///
