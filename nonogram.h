/******************************************************************************
 * Nonogram                                                                   *
 ******************************************************************************/
/******************************************************************************
 This file is part of AmiNonogram.
 Copyright (C) 2022 Ibrahim Alper Sönmez
******************************************************************************/

//Public Attributes
#define MUIA_Nonogram_Data       0x80440001 //(I..) mandatory
#define MUIA_Nonogram_Faults     0x80440002 //(..G) listenable
#define MUIA_Nonogram_Complete   0x80440003 //(..G) listenable

//Public Methods               ex: #define MUIM_Nonogram_{Method}    0x80440001

//Public Functions
struct MUI_CustomClass* MUI_Create_Nonogram(void);
