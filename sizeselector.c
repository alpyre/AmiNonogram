/******************************************************************************
 * SizeSelector                                                               *
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
#include <proto/exec.h>
#include <proto/utility.h>          // <-- Required for tag redirection

#include <libraries/mui.h>
#include <proto/muimaster.h>
#include <clib/alib_protos.h>       // <-- Required for DoSuperMethod()

#include <SDI_compiler.h>           //     Required for
#include <SDI_hook.h>               // <-- multi platform
#include <SDI_stdarg.h>             //     compatibility

#include "dosupernew.h"
#include "sizeselector.h"
///
///Structs
struct cl_ObjTable
{
  Object* text;
  Object* plus;
  Object* minus;
};

struct cl_Data
{
  struct cl_ObjTable obj_table;
  ULONG value;
};

struct cl_Msg
{
  ULONG MethodID;
  //<SUBCLASS METHOD MESSAGE PAYLOAD HERE>
};
///

//<SUBCLASS METHODS>
///m_SetValue()
VOID m_SetValue(struct IClass* cl, Object* obj, UBYTE value) {
  struct cl_Data *data = INST_DATA(cl, obj);
  UBYTE buf[16];

  sprintf(buf, "%lux%lu", (ULONG)value, (ULONG)value);
  DoMethod(data->obj_table.text, MUIM_Set, MUIA_Text_Contents, buf);
  data->value = value;
}
///

///Overridden OM_NEW
static ULONG m_New(struct IClass* cl, Object* obj, struct opSet* msg)
{
  struct cl_Data *data;
  Object* text;
  Object* plus;
  Object* minus;

  if ((obj = (Object *) DoSuperNew(cl, obj,
    MUIA_Group_Horiz, TRUE,
    MUIA_Group_HorizSpacing, 1,
    MUIA_Group_Child, (text = MUI_NewObject(MUIC_Text,
      MUIA_Text_Contents, "5x5",
      MUIA_Frame, MUIV_Frame_Text,
    TAG_END)),
    MUIA_Group_Child, (plus = MUI_NewObject(MUIC_Text,
      MUIA_InputMode, MUIV_InputMode_RelVerify,
      MUIA_Frame, MUIV_Frame_Button,
      MUIA_Background, MUII_ButtonBack,
      MUIA_Font, MUIV_Font_Button,
      MUIA_Text_Contents, "+",
      MUIA_FixWidthTxt, "+",
    TAG_END)),
    MUIA_Group_Child, (minus = MUI_NewObject(MUIC_Text,
      MUIA_InputMode, MUIV_InputMode_RelVerify,
      MUIA_Frame, MUIV_Frame_Button,
      MUIA_Background, MUII_ButtonBack,
      MUIA_Font, MUIV_Font_Button,
      MUIA_Text_Contents, "-",
      MUIA_FixWidthTxt, "-",
    TAG_END)),
    TAG_MORE, msg->ops_AttrList)))
  {
    data = (struct cl_Data *) INST_DATA(cl, obj);
    data->obj_table.text = text;
    data->obj_table.plus = plus;
    data->obj_table.minus = minus;
    data->value = 5;

    DoMethod(plus, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_SizeSelector_Increase);
    DoMethod(minus, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_SizeSelector_Decrease);

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
  //struct cl_Data *data = INST_DATA(cl, obj);
  struct TagItem *tags, *tag;

  for (tags = msg->ops_AttrList; (tag = NextTagItem(&tags));)
  {
    switch (tag->ti_Tag)
    {
      case MUIA_SizeSelector_Value:
      {
        ULONG value = tag->ti_Data;
        if (value && !(value % 5) && value <= MAX_PUZZLE_SIZE) {
          m_SetValue(cl, obj, value);
        }
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
    case MUIA_SizeSelector_Value:
      *msg->opg_Storage = data->value;
    return TRUE;
  }

  return (DoSuperMethodA(cl, obj, (Msg) msg));
}
///
///Dispatcher
SDISPATCHER(cl_Dispatcher)
{
  struct cl_Data *data = NULL;
  if (! (msg->MethodID == OM_NEW)) data = INST_DATA(cl, obj);

  switch(msg->MethodID)
  {
    case OM_NEW:
      return m_New(cl, obj, (struct opSet*) msg);
    case OM_SET:
      return m_Set(cl, obj, (struct opSet*) msg);
    case OM_GET:
      return m_Get(cl, obj, (struct opGet*) msg);
    case MUIM_SizeSelector_Increase:
      return DoMethod(obj, MUIM_Set, MUIA_SizeSelector_Value, data->value + 5);
    case MUIM_SizeSelector_Decrease:
      return DoMethod(obj, MUIM_Set, MUIA_SizeSelector_Value, data->value - 5);

    //<DISPATCH SUBCLASS METHODS HERE>

    default:
      return DoSuperMethodA(cl, obj, msg);
  }
}
///
///Class Creator
struct MUI_CustomClass* MUI_Create_SizeSelector(void)
{
    return (MUI_CreateCustomClass(NULL, MUIC_Group, NULL, sizeof(struct cl_Data), ENTRY(cl_Dispatcher)));
}
///
