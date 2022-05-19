/******************************************************************************
 * Nonogram                                                                   *
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

///Definitions
#define COLUMN 1
#define ROW    0

#define EMPTY  0
#define FULL   1
#define MARKED_EMPTY 2
#define MARKED_FULL  3
#define REVEALED     MARKED_EMPTY //use with <

#define _between(a,x,b) ((x)>=(a) && (x)<=(b))
#define _isinobject(obj,x,y) (_between(_mleft(obj),(x),_mright(obj)) && _between(_mtop(obj),(y),_mbottom(obj)))
#define _squareColumn(obj,s,x) ((((x)-_mleft(obj))*(s))/(_mright(obj)-_mleft(obj) + 1))
#define _squareRow(obj,s,y) ((((y)-_mtop (obj))*(s))/(_mbottom(obj)-_mtop(obj) + 1))
///
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
#include "nonogram.h"
#include "hintdisplay.h"
#include "square.h"
#include "game.h"
///
///Globals
#if defined(__amigaos4__) || defined(__MORPHOS__) || defined(MUI5)
#define MUIM_Square_Event     0x80440100
Object* g_obj_parent;

struct cl_SquareMsg
{
  ULONG MethodID;
  Object *square;
};
#endif
///
///Structs
struct cl_ObjTable
{
  Object *modeDisplay;
  Object *horizontalHintGroup;
  Object *verticalHintGroup;
  Object *playAreaGroup;
};

struct cl_Data
{
  struct cl_ObjTable obj_table;
  GameBoardData *gameBoardData;
  Object** horizontalHintDisplays;
  Object** verticalHintDisplays;
  Object** squares;
  ULONG mode;     //Square mark mode: FULL or EMPTY
  ULONG faults;   //Number of incorrect inferecences the player made
  ULONG revealed; //Number of rows solved by the player so far
  ULONG complete; //BOOL: Becomes true when player solves the nonogram completely
};

struct cl_Msg
{
  ULONG MethodID;
  //<SUBCLASS METHOD MESSAGE PAYLOAD HERE>
};
///
///Globals
extern APTR memoryPool;
extern struct MUI_CustomClass *cl_HintsDisplay;
extern struct MUI_CustomClass *cl_Square;
///

//<SUBCLASS PRIVATE FUNCTIONS>
///itoa(number, buffer, len)
/******************************************************************************
 * A custom itoa implementation only to be used by this class.                *
 ******************************************************************************/
VOID itoa(ULONG number, STRPTR str, ULONG len) {
  UBYTE digit;
  UBYTE *cursor = str + len;
  *cursor = NULL;

  if (number == 0) {
    *(--cursor) = (UBYTE)'0';
  }

  while (number) {
    digit = number % 10;
    *(--cursor) = digit + (UBYTE)'0';
    number /= 10;
  }
}
///
///digitCount(number)
/******************************************************************************
 * Returns the number of decimal digits of the given integer.                 *
 ******************************************************************************/
ULONG digitCount(ULONG number) {
  ULONG result = 0;

  if (number == 0) {
    return 1;
  }

  while (number) {
    number /= 10;
    result++;
  }

  return result;
}
///
///getHintsArray(gameBoardData, orientation, row_column)
/******************************************************************************
 * Returns the hints as a NULL terminated array of strings from the given row *
 * or column of a gameBoardData.                                              *
 ******************************************************************************/
STRPTR *getHintsArray(GameBoardData *gbd, ULONG orientation, ULONG number) {
  STRPTR *result = NULL;
  ULONG buffer[32] = {0}; //WARNING: Possible overflow here!!
  ULONG hints = 0;
  ULONG hint = 0;
  ULONG i;
  STRPTR str;
  ULONG len;
  BOOL found = FALSE;

  if (orientation == COLUMN) {
    ULONG r;

    for (r = 0; r < gbd->size; r++) {
      if (gbd->data[r * gbd->size + number] == 1) {
        found = TRUE;
        buffer[hint]++;
      }
      else if (found == TRUE) {
        hint++;
        found = FALSE;
      }
    }
  }
  else // a ROW
  {
    ULONG s = number * gbd->size;
    ULONG c;

    for (c = s; c < s+gbd->size; c++) {
      if (gbd->data[c] == 1) {
        found = TRUE;
        buffer[hint]++;
      }
      else if (found == TRUE) {
        hint++;
        found = FALSE;
      }
    }
  }

  hints = hint + (found ? 1 : 0);
  result = AllocPooled(memoryPool, sizeof(STRPTR) * (hints + 1));
  if (result) {
    for (i = 0; i < hints; i++) {
      len = digitCount(buffer[i]);
      str = AllocPooled(memoryPool, len + 1);
      if (str) {
        itoa(buffer[i], str, len);
        result[i] = str;
      }
      else {
        //TODO: code a proper fail state here!
        printf("We're going down!\n");
        break;
      }
    }
    result[hints] = NULL;
  }

  return result;
}
///
///freeHintsArray(arr)
VOID freeHintsArray(STRPTR* arr) {
  ULONG i = 0;
  STRPTR str;

  while ((str = arr[i++])) {
    FreePooled(memoryPool, str, strlen(str) + 1);
  }
  FreePooled(memoryPool, arr, sizeof(STRPTR) * i);
}
///
///hintDisplayGroup(gameBoardData, orientation, &array)
static Object *hintDisplayGroup(GameBoardData *gbd, ULONG orientation, Object*** array) {
  Object *object;
  Object **hintDisplays;
  struct TagItem *tagList;
  struct TagItem *tag;
  ULONG i;

  hintDisplays = (Object**)AllocPooled(memoryPool, sizeof(Object*) * gbd->size);
  if (hintDisplays) {
    tagList = (struct TagItem*)AllocPooled(memoryPool, sizeof(struct TagItem) * (gbd->size + 1));
    if (tagList) {
      tag = tagList;

      for (i = 0; i < gbd->size; i++) {
        STRPTR *hintsArray = getHintsArray(gbd, orientation, i);

        hintDisplays[i] = NewObject(cl_HintsDisplay->mcc_Class, NULL, MUIA_HintDisplay_HintArray, hintsArray,
                                                                      MUIA_Group_Horiz, !orientation,
                                                                      TAG_END);
        if (!hintDisplays[i]) goto error1;
        freeHintsArray(hintsArray);

        tag->ti_Tag  = MUIA_Group_Child;
        tag->ti_Data = (ULONG)hintDisplays[i];
        tag++;
      }
      tag->ti_Tag  = TAG_END;
      tag->ti_Data = NULL;

      object = MUI_NewObject(MUIC_Group, MUIA_Group_Horiz, orientation,
                                         MUIA_Group_SameSize, TRUE,
                                         MUIA_Group_Spacing, 2,
                                         MUIA_InnerLeft, orientation ? 2 : 0,
                                         MUIA_InnerRight, orientation ? 2 : 0,
                                         TAG_MORE, (ULONG)tagList);

      if (object) {
        FreePooled(memoryPool, tagList, sizeof(struct TagItem) * (gbd->size + 1));
        *array = hintDisplays;

        return object;
      }
    }
    else {
      FreePooled(memoryPool, hintDisplays, sizeof(Object*) * gbd->size);
      return NULL;
    }
  }
  else return NULL;

error1:
  for (i = 0; i < gbd->size && hintDisplays[i]; i++) {
    DisposeObject(hintDisplays[i]);
  }
  FreePooled(memoryPool, tagList, sizeof(struct TagItem) * (gbd->size + 1));
  FreePooled(memoryPool, hintDisplays, sizeof(Object*) * gbd->size);
  return NULL;
}
///
///playAreaGroup(size, &squares_array)
Object* playAreaGroup(ULONG size, Object*** squares_array) {
  Object *playArea;
  ULONG i;
  struct TagItem *tagList;
  struct TagItem *tag;
  ULONG num_squares = size * size;
  ULONG num_columns = size / 5;
  ULONG num_cells = num_columns * num_columns;
  Object **squares;
  Object **cells;

  squares = AllocPooled(memoryPool, sizeof(Object*) * (num_squares));
  if (squares) {
    struct TagItem tags[26];

    for (i = 0; i < num_squares; i++) {
      squares[i] = NewObject(cl_Square->mcc_Class, NULL, MUIA_Background, MUII_SHINE, TAG_END);
      if (!squares[i]) goto error2;
    }

    cells = AllocPooled(memoryPool, sizeof(Object*) * num_cells);
    if (cells) {
      for (i = 0; i < num_cells; i++) {
        ULONG l;
        for (l = 0; l < 25; l++) {
          //             column    row                cell column offset        cell row offset
          ULONG index = (l % 5) + ((l / 5) * size) + ((i % num_columns) * 5) + ((i / num_columns) * size * 5) ;
          tags[l].ti_Tag  = MUIA_Group_Child;
          tags[l].ti_Data = (ULONG)squares[index];
        }
        tags[25].ti_Tag  = TAG_END;
        tags[25].ti_Data = NULL;

        cells[i] = MUI_NewObject(MUIC_Group, MUIA_Group_Columns, 5,
                                             MUIA_Group_Spacing, 2,
                                             MUIA_Background, MUII_SHADOWBACK,
                                             TAG_MORE, &tags);
        if (!cells[i]) {
          ULONG j;
          for (j = 0; j < i; j++) MUI_DisposeObject(cells[j]);
          for (j = i + 1; j < num_cells; j++) {
            for (l = 0; l < 25; l++) {
              ULONG index = (l % 5) + ((l / 5) * size) + ((j % num_columns) * 5) + ((j / num_columns) * size * 5) ;
              MUI_DisposeObject(squares[index]);
            }
          }
          goto error4;
        }
      }

      tagList = AllocPooled(memoryPool, sizeof(struct TagItem) * (num_cells + 1));
      if (tagList) {
        tag = tagList;
        for (i = 0; i < num_cells; i++) {
          tag->ti_Tag  = MUIA_Group_Child;
          tag->ti_Data = (ULONG)cells[i];
          tag++;
        }
        tag->ti_Tag  = TAG_END;
        tag->ti_Data = NULL;

        playArea = MUI_NewObject(MUIC_Group, MUIA_Group_Columns, num_columns,
                                             MUIA_Background, MUII_SHADOW,
                                             MUIA_Group_Spacing, 2,
                                             MUIA_InnerTop, 2,
                                             MUIA_InnerBottom, 2,
                                             MUIA_InnerLeft, 2,
                                             MUIA_InnerRight, 2,
                                             TAG_MORE, tagList);
        FreePooled(memoryPool, tagList, sizeof(struct TagItem) * (num_cells + 1));
        if (playArea) {
          FreePooled(memoryPool, cells, sizeof(Object*) * (num_cells));
          *squares_array = squares;
          return playArea;
        }
        else goto error4;
      }
      else goto error3;
    }
    else goto error2;
  }
  else return NULL;

error3:
  for (i = 0; i < num_cells; i++) {
    MUI_DisposeObject(cells[i]);
  }
error4:
  FreePooled(memoryPool, cells, sizeof(Object*) * (num_cells));
  FreePooled(memoryPool, squares, sizeof(Object*) * (num_squares));
  squares = NULL;
  return NULL;

error2:
  for (i = 0; i < num_squares && squares[i]; i++) {
    DisposeObject(squares[i]);
  }
  FreePooled(memoryPool, squares, sizeof(Object*) * (num_squares));
  squares = NULL;
  return NULL;
}
///
///checkCompleteness(data, row, column, orientation)
VOID checkCompleteness(struct cl_Data* data, ULONG row, ULONG column, ULONG orientation) {
  ULONG i, j, s;
  ULONG size = data->gameBoardData->size;
  ULONG unrevealedFull = 0;
  ULONG found = FALSE;
  ULONG hint = 0;
  Object *hintDisplay = orientation == ROW ? data->horizontalHintDisplays[row] : data->verticalHintDisplays[column];
  ULONG incr = orientation == ROW ? 1 : size;

  s = orientation == ROW ? row * size : column;
  for (i = 0; i < size; i++) {
    if (data->gameBoardData->data[s] == MARKED_FULL) found = TRUE;
    if (found == TRUE && (data->gameBoardData->data[s] == EMPTY || data->gameBoardData->data[s] == MARKED_EMPTY)) {
      DoMethod(hintDisplay, MUIM_HintDisplay_Reveal, hint, FALSE);
      found = FALSE;
      hint++;
    }
    if (data->gameBoardData->data[s] < REVEALED) {
      found = FALSE;
      hint = 0;
      if (data->gameBoardData->data[s] == FULL) unrevealedFull++;

      s = orientation == ROW ? ((row + 1) * size) - 1 : ((size - 1) * size) + column;
      for (j = size - 1; j > i; j--) {
        if (data->gameBoardData->data[s] == MARKED_FULL) found = TRUE;
        if (found == TRUE && (data->gameBoardData->data[s] == EMPTY || data->gameBoardData->data[s] == MARKED_EMPTY)) {
          DoMethod(hintDisplay, MUIM_HintDisplay_Reveal, hint, TRUE);
          found = FALSE;
          hint++;
        }
        if (data->gameBoardData->data[s] < REVEALED) {
          if (data->gameBoardData->data[s] == FULL) unrevealedFull++;
          if (unrevealedFull) break;
        }
        s -= incr;
      }
      break;
    }
    s += incr;
  }
  if (!unrevealedFull) {
    //Completely reveal this row/column
    if (orientation == ROW) {
      data->revealed++;
      s = row * size;
    }
    else
      s = column;

    for (i = 0; i < size; i++) {
      if (data->gameBoardData->data[s] < REVEALED) {
        DoMethod(data->squares[s], MUIM_Set, MUIA_Square_State, data->gameBoardData->data[s] == FULL ? SQUARE_SELECTED : SQUARE_MARKED);
        data->gameBoardData->data[s] += MARKED_EMPTY;
      }
      s += incr;
    }
    DoMethod(hintDisplay, MUIM_HintDisplay_RevealAll);
  }
}
///

///Overridden OM_NEW
static ULONG m_New(struct IClass* cl, Object* obj, struct opSet* msg)
{
  struct cl_Data *data;
  struct TagItem *tag;
  GameBoardData *gameBoardData;
  Object** horizontalHintDisplays = NULL;
  Object** verticalHintDisplays = NULL;
  Object** squares = NULL;
  Object* modeDisplay;
  Object* pag;

  if ((tag = FindTagItem(MUIA_Nonogram_Data, msg->ops_AttrList)) &&
      (gameBoardData = (GameBoardData*)tag->ti_Data) &&
      gameBoardData->size && gameBoardData->size % 5 == 0 &&
      gameBoardData->data) {

  }
  else return NULL;

  if ((obj = (Object *) DoSuperNew(cl, obj,
    MUIA_ContextMenu, TRUE, // To be able to capture right mouse click events
    MUIA_Group_Columns, 2,
    MUIA_Group_Spacing, 2,
    MUIA_Group_Child, MUI_NewObject(MUIC_Group,
                      MUIA_Group_Horiz, TRUE,
                      MUIA_Group_Child, MUI_NewObject(MUIC_Rectangle, TAG_END),
                      MUIA_Group_Child, MUI_NewObject(MUIC_Group,
                                        MUIA_Group_Child, MUI_NewObject(MUIC_Rectangle, TAG_END),
                                        MUIA_Group_Child, MUI_NewObject(MUIC_Group,
                                                          MUIA_Background, MUII_SHADOW,
                                                          MUIA_Group_Spacing, 2,
                                                          MUIA_InnerTop, 2,
                                                          MUIA_InnerBottom, 2,
                                                          MUIA_InnerLeft, 2,
                                                          MUIA_InnerRight, 2,
                                                          MUIA_Group_Child, (modeDisplay = NewObject(cl_Square->mcc_Class, NULL,
                                                                                           MUIA_FixWidth  , 13,
                                                                                           MUIA_FixHeight , 8,
                                                                                           MUIA_Square_State, SQUARE_SELECTED,
                                                                                           TAG_END)),
                                                          TAG_END),
                                        MUIA_Group_Child, MUI_NewObject(MUIC_Rectangle, TAG_END),
                                        TAG_END),
                      MUIA_Group_Child, MUI_NewObject(MUIC_Rectangle, TAG_END),
                      TAG_END),
    MUIA_Group_Child, hintDisplayGroup(gameBoardData, COLUMN, &verticalHintDisplays),
    MUIA_Group_Child, hintDisplayGroup(gameBoardData, ROW, &horizontalHintDisplays),
    MUIA_Group_Child, (pag = playAreaGroup(gameBoardData->size, &squares)),
    TAG_MORE, msg->ops_AttrList)))
  {
    data = (struct cl_Data*) INST_DATA(cl, obj);
    data->gameBoardData = gameBoardData;
    data->horizontalHintDisplays = horizontalHintDisplays;
    data->verticalHintDisplays = verticalHintDisplays;
    data->squares = squares;
    data->obj_table.playAreaGroup = pag;
    data->obj_table.modeDisplay = modeDisplay;
    data->mode = FULL;
    data->faults = 0;
    data->revealed = 0;
    data->complete = FALSE;

    #if defined(__amigaos4__) || defined(__MORPHOS__) || defined(MUI5)
    g_obj_parent = obj;
    #endif

    //<SUBCLASS INITIALIZATION HERE>
    if (/*<Success of your initializations>*/ TRUE) {

      return((ULONG) obj);
    }
    else CoerceMethod(cl, obj, OM_DISPOSE);
  }
  else {
    if (horizontalHintDisplays) FreePooled(memoryPool, horizontalHintDisplays, sizeof(Object*) * gameBoardData->size);
    if (verticalHintDisplays) FreePooled(memoryPool, verticalHintDisplays, sizeof(Object*) * gameBoardData->size);
  }

  return NULL;
}
///
///Overridden OM_DISPOSE
static ULONG m_Dispose(struct IClass* cl, Object* obj, struct opSet* msg)
{
  struct cl_Data* data = INST_DATA(cl, obj);

  if (data->horizontalHintDisplays) {
    FreePooled(memoryPool, data->horizontalHintDisplays, sizeof(Object*) * data->gameBoardData->size);
  }
  if (data->verticalHintDisplays) {
    FreePooled(memoryPool, data->verticalHintDisplays, sizeof(Object*) * data->gameBoardData->size);
  }
  if (data->squares) {
    FreePooled(memoryPool, data->squares, sizeof(Object*) * (data->gameBoardData->size * data->gameBoardData->size));
  }

  return DoSuperMethodA(cl, obj, (Msg) msg);
}
///
///Overridden OM_SET
//*****************
static ULONG m_Set(struct IClass* cl, Object* obj, struct opSet* msg)
{
  /*
  struct cl_Data* data = INST_DATA(cl, obj);
  struct TagItem *tags, *tag;

  for (tags = msg->ops_AttrList; (tag = NextTagItem(&tags));)
  {
    switch (tag->ti_Tag)
    {
      //<SUBCLASS ATTRIBUTES HERE>
    }
  }
  */
  return (DoSuperMethodA(cl, obj, (Msg) msg));
}
///
///Overridden OM_GET
//*****************
static ULONG m_Get(struct IClass* cl, Object* obj, struct opGet* msg)
{
  struct cl_Data* data = INST_DATA(cl, obj);

  switch (msg->opg_AttrID)
  {
    case MUIA_Nonogram_Faults:
      *msg->opg_Storage = data->faults;
    return TRUE;
    case MUIA_Nonogram_Complete:
      *msg->opg_Storage = data->complete;
    return TRUE;
  }

  return (DoSuperMethodA(cl, obj, (Msg) msg));
}
///
///Overridden HandleInput TODO: Refactor!
static ULONG m_HandleInput(struct IClass *cl, Object *obj, struct MUIP_HandleInput *msg)
{
  struct cl_Data* data = INST_DATA(cl, obj);

  if (msg->imsg) {
    switch (msg->imsg->Class) {
      case IDCMP_MOUSEBUTTONS:
        switch (msg->imsg->Code) {
          case SELECTDOWN:
            if (_isinobject(data->obj_table.playAreaGroup, msg->imsg->MouseX, msg->imsg->MouseY)) {
              ULONG row = _squareRow(data->obj_table.playAreaGroup, data->gameBoardData->size, msg->imsg->MouseY);
              ULONG column = _squareColumn(data->obj_table.playAreaGroup, data->gameBoardData->size, msg->imsg->MouseX);
              ULONG square = column + row * data->gameBoardData->size;

              if (data->gameBoardData->data[square] < MARKED_EMPTY) {
                if (data->gameBoardData->data[square] == data->mode) // Correct inference
                {
                  data->gameBoardData->data[square] = data->mode + MARKED_EMPTY;
                  DoMethod(data->squares[square], MUIM_Set, MUIA_Square_State, data->mode == FULL ? SQUARE_SELECTED : SQUARE_MARKED);
                  MUI_RequestIDCMP(obj, IDCMP_MOUSEMOVE);
                }
                else // Fault
                {
                  DoMethod(data->squares[square], MUIM_Set, MUIA_Square_State, data->gameBoardData->data[square] == FULL ? SQUARE_SELECTED : SQUARE_MARKED);
                  data->gameBoardData->data[square] += MARKED_EMPTY;
                  data->faults++;
                  DoMethod(obj, MUIM_Set, MUIA_Nonogram_Faults, data->faults); // Raise a notification on this attribute
                }

                // Update hint displays and auto-reveal for this row and column
                checkCompleteness(data, row, column, ROW);
                checkCompleteness(data, row, column, COLUMN);

                if (data->revealed == data->gameBoardData->size) {
                  //Raise a notification on Complete attribute
                  data->complete = TRUE;
                  DoMethod(obj, MUIM_Set, MUIA_Nonogram_Complete, TRUE);
                }
              }
            }
          break;

          case SELECTUP:
            MUI_RejectIDCMP(obj, IDCMP_MOUSEMOVE);
          break;
        }
      break;

      case IDCMP_MOUSEMOVE:
        if (_isinobject(data->obj_table.playAreaGroup, msg->imsg->MouseX, msg->imsg->MouseY)) {
          ULONG row = _squareRow(data->obj_table.playAreaGroup, data->gameBoardData->size, msg->imsg->MouseY);
          ULONG column = _squareColumn(data->obj_table.playAreaGroup, data->gameBoardData->size, msg->imsg->MouseX);
          ULONG square = column + row * data->gameBoardData->size;

          if (data->gameBoardData->data[square] < MARKED_EMPTY) {
            if (data->gameBoardData->data[square] == data->mode) // Correct inference
            {
              data->gameBoardData->data[square] = data->mode + MARKED_EMPTY;
              DoMethod(data->squares[square], MUIM_Set, MUIA_Square_State, data->mode == FULL ? SQUARE_SELECTED : SQUARE_MARKED);
            }
            else // Fault
            {
              DoMethod(data->squares[square], MUIM_Set, MUIA_Square_State, data->gameBoardData->data[square] == FULL ? SQUARE_SELECTED : SQUARE_MARKED);
              data->gameBoardData->data[square] += MARKED_EMPTY;
              MUI_RejectIDCMP(obj, IDCMP_MOUSEMOVE);
              data->faults++;
              DoMethod(obj, MUIM_Set, MUIA_Nonogram_Faults, data->faults); // Raise a notification on this attribute
            }
            // Update hint displays and auto-reveal for this row and column
            checkCompleteness(data, row, column, ROW);
            checkCompleteness(data, row, column, COLUMN);

            if (data->revealed == data->gameBoardData->size) {
              //Raise a notification on Complete attribute
              data->complete = TRUE;
              DoMethod(obj, MUIM_Set, MUIA_Nonogram_Complete, TRUE);
            }
          }
        }
      break;
    }
  }

  return(0);
}
///
///m_RightClickEvent
static ULONG m_RightClickEvent(struct IClass *cl, Object *obj, Msg msg) {
  struct cl_Data* data = INST_DATA(cl,obj);

  //Toggle Mode
  switch (data->mode) {
    case EMPTY:
      data->mode = FULL;
      DoMethod(data->obj_table.modeDisplay, MUIM_Set, MUIA_Square_State, SQUARE_SELECTED);
    break;
    case FULL:
      data->mode = EMPTY;
      DoMethod(data->obj_table.modeDisplay, MUIM_Set, MUIA_Square_State, SQUARE_MARKED);
    break;
  }

  return (ULONG)NULL;
}
///
///m_Square_Event
#if defined(__amigaos4__) || defined(__MORPHOS__) || defined(MUI5)
ULONG m_Square_Event(struct IClass *cl, Object *obj, Msg msg) {
  struct cl_Data* data = INST_DATA(cl, obj);
  Object *obj_square = ((struct cl_SquareMsg*)msg)->square;
  ULONG num_Squares = data->gameBoardData->size * data->gameBoardData->size;
  ULONG square;
  ULONG row;
  ULONG column;

  for (square = 0; square < num_Squares; square++) {
    if (data->squares[square] == obj_square) break;
  }
  if (square == num_Squares) return 0;
  column = square % data->gameBoardData->size;
  row = square / data->gameBoardData->size;

  if (data->gameBoardData->data[square] < MARKED_EMPTY) {
    if (data->gameBoardData->data[square] == data->mode) // Correct inference
    {
      data->gameBoardData->data[square] = data->mode + MARKED_EMPTY;
      DoMethod(data->squares[square], MUIM_Set, MUIA_Square_State, data->mode == FULL ? SQUARE_SELECTED : SQUARE_MARKED);
      MUI_RequestIDCMP(obj, IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE);
    }
    else // Fault
    {
      DoMethod(data->squares[square], MUIM_Set, MUIA_Square_State, data->gameBoardData->data[square] == FULL ? SQUARE_SELECTED : SQUARE_MARKED);
      data->gameBoardData->data[square] += MARKED_EMPTY;
      data->faults++;
      DoMethod(obj, MUIM_Set, MUIA_Nonogram_Faults, data->faults); // Raise a notification on this attribute
    }

    // Update hint displays and auto-reveal for this row and column
    checkCompleteness(data, row, column, ROW);
    checkCompleteness(data, row, column, COLUMN);

    if (data->revealed == data->gameBoardData->size) {
      //Raise a notification on Complete attribute
      data->complete = TRUE;
      DoMethod(obj, MUIM_Set, MUIA_Nonogram_Complete, TRUE);
    }
  }
}
#endif
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
      return m_Dispose(cl, obj, (struct opSet*) msg);
    case OM_SET:
      return m_Set(cl, obj, (struct opSet*) msg);
    case OM_GET:
      return m_Get(cl, obj, (struct opGet*) msg);
    case MUIM_HandleInput:
      return m_HandleInput(cl, obj, (struct MUIP_HandleInput*) msg);
    case MUIM_ContextMenuBuild:
      return m_RightClickEvent(cl, obj, (Msg) msg);
    #if defined(__amigaos4__) || defined(__MORPHOS__) || defined(MUI5)
    case MUIM_Square_Event:
      return m_Square_Event(cl, obj, msg);
    #endif

    //<DISPATCH SUBCLASS METHODS HERE>

    default:
      return DoSuperMethodA(cl, obj, msg);
  }
}
///
///Class Creator
struct MUI_CustomClass* MUI_Create_Nonogram(void)
{
    return (MUI_CreateCustomClass(NULL, MUIC_Group, NULL, sizeof(struct cl_Data), ENTRY(cl_Dispatcher)));
}
///
