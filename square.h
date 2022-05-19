/******************************************************************************
 * Square                                                                     *
 ******************************************************************************/
/******************************************************************************
 This file is part of AmiNonogram.
 Copyright (C) 2022 Ibrahim Alper Sönmez
******************************************************************************/

#define SQUARE_UNSELECTED 0
#define SQUARE_SELECTED 1
#define SQUARE_MARKED 2

//Public Attributes
#define MUIA_Square_State       0x80450001 //(IS.)

//Public Methods                 ex: #define MUIM_Square_{Method}    0x80450001

//Public Functions
struct MUI_CustomClass* MUI_Create_Square(void);
