/******************************************************************************
 * Square                                                                     *
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

#include <libraries/mui.h>
#include <proto/muimaster.h>
#include <clib/alib_protos.h> // <-- Required for DoSuperMethod()

#include <SDI_compiler.h>     //     Required for
#include <SDI_hook.h>         // <-- multi platform
#include <SDI_stdarg.h>       //     compatibility

#include "dosupernew.h"
#include "square.h"
///
///Globals&Definitions
#if defined(__amigaos4__) || defined(__MORPHOS__) || defined(MUI5)
#define MUIM_Square_Event     0x80440100
extern Object* g_obj_parent;
#endif
///
///Structs
struct cl_Data
{
  ULONG state;
};
///

///Overridden OM_NEW
static ULONG m_New(struct IClass* cl, Object* obj, struct opSet* msg)
{
  struct cl_Data *data;
  struct TagItem *tag;
  ULONG state = SQUARE_UNSELECTED;

  if ((tag = FindTagItem(MUIA_Square_State, msg->ops_AttrList))) {
    state = tag->ti_Data;
  }

  if ((obj = (Object *) DoSuperNew(cl, obj, MUIA_InputMode, MUIV_InputMode_Immediate,
                                            MUIA_Background, state == SQUARE_SELECTED ? MUII_FILL : MUII_SHINE,
                                            MUIA_FixWidthTxt , "00",
                                            state == SQUARE_MARKED ? MUIA_Text_Contents : TAG_IGNORE, "\33cX",
                                            TAG_MORE, msg->ops_AttrList)))
  {
    data = (struct cl_Data*) INST_DATA(cl, obj);
    data->state = state;

    //<SUBCLASS INITIALIZATION HERE>
    if (/*<Success of your initializations>*/ TRUE) {

      return((ULONG) obj);
    }
    else CoerceMethod(cl, obj, OM_DISPOSE);
  }

return NULL;
}
///
///Overridden OM_SET
//*****************
static ULONG m_Set(struct IClass* cl, Object* obj, struct opSet* msg)
{
  struct cl_Data* data = INST_DATA(cl, obj);
  struct TagItem *tags, *tag;

  for (tags = msg->ops_AttrList; (tag = NextTagItem(&tags));)
  {
    switch (tag->ti_Tag)
    {
      case MUIA_Square_State:
        data->state = tag->ti_Data;
        switch (data->state) {
          case SQUARE_SELECTED:
            DoMethod(obj, MUIM_Set, MUIA_Background, MUII_FILL);
            DoMethod(obj, MUIM_Set, MUIA_Text_Contents, "");
          break;
          case SQUARE_MARKED:
            DoMethod(obj, MUIM_Set, MUIA_Background, MUII_SHINE);
            DoMethod(obj, MUIM_Set, MUIA_Text_Contents, "\33cX");
          break;
          case SQUARE_UNSELECTED:
            DoMethod(obj, MUIM_Set, MUIA_Background, MUII_SHINE);
            DoMethod(obj, MUIM_Set, MUIA_Text_Contents, "");
          break;
        }
      return 0;
      #if defined(__amigaos4__) || defined(__MORPHOS__) || defined(MUI5)
      case MUIA_Selected:
        DoMethod(g_obj_parent, MUIM_Square_Event, obj);
        tag->ti_Tag = TAG_IGNORE;
      break;
      #endif
    }
  }

  return (DoSuperMethodA(cl, obj, (Msg) msg));
}
///
///Overridden HandleInput
static ULONG m_HandleInput(struct IClass *cl, Object *obj, struct MUIP_HandleInput *msg) {
  //struct cl_Data* data = INST_DATA(cl,obj);

  //This object does not handle input by itself

  return(0);
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
    case OM_SET:
      return m_Set(cl, obj, (struct opSet*) msg);
    case MUIM_HandleInput:
      return m_HandleInput(cl, obj, (struct MUIP_HandleInput*) msg);

    //<DISPATCH SUBCLASS METHODS HERE>

    default:
      return DoSuperMethodA(cl, obj, msg);
  }
}
///
///Class Creator
struct MUI_CustomClass* MUI_Create_Square(void)
{
    return (MUI_CreateCustomClass(NULL, MUIC_Text, NULL, sizeof(struct cl_Data), ENTRY(cl_Dispatcher)));
}
///
