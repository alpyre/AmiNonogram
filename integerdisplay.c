/******************************************************************************
 * IntegerDisplay                                                             *
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
#include "integerdisplay.h"
///
///Structs
struct cl_Data
{
  LONG value;
};
///

///Overridden OM_NEW
static ULONG m_New(struct IClass* cl, Object* obj, struct opSet* msg)
{
  UBYTE buffer[16];
  struct cl_Data *data;
  LONG value;

  value = (LONG) GetTagData(MUIA_IntegerDisplay_Value, 0, msg->ops_AttrList);
  sprintf(buffer, "%ld", value);

  if ((obj = (Object *) DoSuperNew(cl, obj,
    MUIA_Text_Contents, buffer,
    TAG_MORE, msg->ops_AttrList)))
  {
    data = (struct cl_Data *) INST_DATA(cl, obj);

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
  struct cl_Data *data = INST_DATA(cl, obj);
  struct TagItem *tags, *tag;
  UBYTE buffer[16];

  for (tags = msg->ops_AttrList; (tag = NextTagItem(&tags));)
  {
    switch (tag->ti_Tag)
    {
      case MUIA_IntegerDisplay_Value:
        data->value = tag->ti_Data;
        sprintf(buffer, "%ld", data->value);
        DoSuperMethod(cl, obj, MUIM_Set, MUIA_Text_Contents, buffer);
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
    case MUIA_IntegerDisplay_Value:
      *msg->opg_Storage = data->value;
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
    case OM_SET:
      return m_Set(cl, obj, (struct opSet*) msg);
    case OM_GET:
      return m_Get(cl, obj, (struct opGet*) msg);
    default:
      return DoSuperMethodA(cl, obj, msg);
  }
}
///
///Class Creator
struct MUI_CustomClass* MUI_Create_IntegerDisplay(void)
{
    return (MUI_CreateCustomClass(NULL, MUIC_Text, NULL, sizeof(struct cl_Data), ENTRY(cl_Dispatcher)));
}
///
